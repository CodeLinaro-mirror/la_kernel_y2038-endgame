/*
 *  linux/drivers/video/fbmem.c
 *
 *  Copyright (C) 1994 Martin Schaller
 *
 *	2001 - Documented with DocBook
 *	- Brad Douglas <brad@neruo.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/module.h>

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vt.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/kmod.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/efi.h>
#include <linux/fb.h>
#include <linux/fbcon.h>
#include <linux/mem_encrypt.h>
#include <linux/pci.h>

#include <asm/fb.h>

#include "fb_device.h"

    /*
     *  Frame buffer device initialization and setup routines
     */

#define FBPIXMAPSIZE	(1024 * 8)

static DEFINE_MUTEX(registration_lock);

struct fb_info *registered_fb[FB_MAX] __read_mostly;
EXPORT_SYMBOL(registered_fb);

int num_registered_fb __read_mostly;
EXPORT_SYMBOL(num_registered_fb);

struct fb_info *get_fb_info(unsigned int idx)
{
	struct fb_info *fb_info;

	if (idx >= FB_MAX)
		return ERR_PTR(-ENODEV);

	mutex_lock(&registration_lock);
	fb_info = registered_fb[idx];
	if (fb_info)
		refcount_inc(&fb_info->count);
	mutex_unlock(&registration_lock);

	return fb_info;
}

void put_fb_info(struct fb_info *fb_info)
{
	if (!refcount_dec_and_test(&fb_info->count))
		return;
	if (fb_info->fbops->fb_destroy)
		fb_info->fbops->fb_destroy(fb_info);
}

/*
 * Helpers
 */

int fb_get_color_depth(struct fb_var_screeninfo *var,
		       struct fb_fix_screeninfo *fix)
{
	int depth = 0;

	if (fix->visual == FB_VISUAL_MONO01 ||
	    fix->visual == FB_VISUAL_MONO10)
		depth = 1;
	else {
		if (var->green.length == var->blue.length &&
		    var->green.length == var->red.length &&
		    var->green.offset == var->blue.offset &&
		    var->green.offset == var->red.offset)
			depth = var->green.length;
		else
			depth = var->green.length + var->red.length +
				var->blue.length;
	}

	return depth;
}
EXPORT_SYMBOL(fb_get_color_depth);

/*
 * Data padding functions.
 */
void fb_pad_aligned_buffer(u8 *dst, u32 d_pitch, u8 *src, u32 s_pitch, u32 height)
{
	__fb_pad_aligned_buffer(dst, d_pitch, src, s_pitch, height);
}
EXPORT_SYMBOL(fb_pad_aligned_buffer);

void fb_pad_unaligned_buffer(u8 *dst, u32 d_pitch, u8 *src, u32 idx, u32 height,
				u32 shift_high, u32 shift_low, u32 mod)
{
	u8 mask = (u8) (0xfff << shift_high), tmp;
	int i, j;

	for (i = height; i--; ) {
		for (j = 0; j < idx; j++) {
			tmp = dst[j];
			tmp &= mask;
			tmp |= *src >> shift_low;
			dst[j] = tmp;
			tmp = *src << shift_high;
			dst[j+1] = tmp;
			src++;
		}
		tmp = dst[idx];
		tmp &= mask;
		tmp |= *src >> shift_low;
		dst[idx] = tmp;
		if (shift_high < mod) {
			tmp = *src << shift_high;
			dst[idx+1] = tmp;
		}
		src++;
		dst += d_pitch;
	}
}
EXPORT_SYMBOL(fb_pad_unaligned_buffer);

/*
 * we need to lock this section since fb_cursor
 * may use fb_imageblit()
 */
char* fb_get_buffer_offset(struct fb_info *info, struct fb_pixmap *buf, u32 size)
{
	u32 align = buf->buf_align - 1, offset;
	char *addr = buf->addr;

	/* If IO mapped, we need to sync before access, no sharing of
	 * the pixmap is done
	 */
	if (buf->flags & FB_PIXMAP_IO) {
		if (info->fbops->fb_sync && (buf->flags & FB_PIXMAP_SYNC))
			info->fbops->fb_sync(info);
		return addr;
	}

	/* See if we fit in the remaining pixmap space */
	offset = buf->offset + align;
	offset &= ~align;
	if (offset + size > buf->size) {
		/* We do not fit. In order to be able to re-use the buffer,
		 * we must ensure no asynchronous DMA'ing or whatever operation
		 * is in progress, we sync for that.
		 */
		if (info->fbops->fb_sync && (buf->flags & FB_PIXMAP_SYNC))
			info->fbops->fb_sync(info);
		offset = 0;
	}
	buf->offset = offset + size;
	addr += offset;

	return addr;
}
EXPORT_SYMBOL(fb_get_buffer_offset);

static void *fb_seq_start(struct seq_file *m, loff_t *pos)
{
	mutex_lock(&registration_lock);
	return (*pos < FB_MAX) ? pos : NULL;
}

static void *fb_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	(*pos)++;
	return (*pos < FB_MAX) ? pos : NULL;
}

static void fb_seq_stop(struct seq_file *m, void *v)
{
	mutex_unlock(&registration_lock);
}

static int fb_seq_show(struct seq_file *m, void *v)
{
	int i = *(loff_t *)v;
	struct fb_info *fi = registered_fb[i];

	if (fi)
		seq_printf(m, "%d %s\n", fi->node, fi->fix.id);
	return 0;
}

static const struct seq_operations __maybe_unused proc_fb_seq_ops = {
	.start	= fb_seq_start,
	.next	= fb_seq_next,
	.stop	= fb_seq_stop,
	.show	= fb_seq_show,
};

int
fb_pan_display(struct fb_info *info, struct fb_var_screeninfo *var)
{
	struct fb_fix_screeninfo *fix = &info->fix;
	unsigned int yres = info->var.yres;
	int err = 0;

	if (var->yoffset > 0) {
		if (var->vmode & FB_VMODE_YWRAP) {
			if (!fix->ywrapstep || (var->yoffset % fix->ywrapstep))
				err = -EINVAL;
			else
				yres = 0;
		} else if (!fix->ypanstep || (var->yoffset % fix->ypanstep))
			err = -EINVAL;
	}

	if (var->xoffset > 0 && (!fix->xpanstep ||
				 (var->xoffset % fix->xpanstep)))
		err = -EINVAL;

	if (err || !info->fbops->fb_pan_display ||
	    var->yoffset > info->var.yres_virtual - yres ||
	    var->xoffset > info->var.xres_virtual - info->var.xres)
		return -EINVAL;

	if ((err = info->fbops->fb_pan_display(var, info)))
		return err;
	info->var.xoffset = var->xoffset;
	info->var.yoffset = var->yoffset;
	if (var->vmode & FB_VMODE_YWRAP)
		info->var.vmode |= FB_VMODE_YWRAP;
	else
		info->var.vmode &= ~FB_VMODE_YWRAP;
	return 0;
}
EXPORT_SYMBOL(fb_pan_display);

static int fb_check_caps(struct fb_info *info, struct fb_var_screeninfo *var,
			 u32 activate)
{
	struct fb_blit_caps caps, fbcaps;
	int err = 0;

	memset(&caps, 0, sizeof(caps));
	memset(&fbcaps, 0, sizeof(fbcaps));
	caps.flags = (activate & FB_ACTIVATE_ALL) ? 1 : 0;
	fbcon_get_requirement(info, &caps);
	info->fbops->fb_get_caps(info, &fbcaps, var);

	if (((fbcaps.x ^ caps.x) & caps.x) ||
	    ((fbcaps.y ^ caps.y) & caps.y) ||
	    (fbcaps.len < caps.len))
		err = -EINVAL;

	return err;
}

int
fb_set_var(struct fb_info *info, struct fb_var_screeninfo *var)
{
	int ret = 0;
	u32 activate;
	struct fb_var_screeninfo old_var;
	struct fb_videomode mode;
	struct fb_event event;
	u32 unused;

	if (var->activate & FB_ACTIVATE_INV_MODE) {
		struct fb_videomode mode1, mode2;

		fb_var_to_videomode(&mode1, var);
		fb_var_to_videomode(&mode2, &info->var);
		/* make sure we don't delete the videomode of current var */
		ret = fb_mode_is_equal(&mode1, &mode2);
		if (!ret) {
			ret = fbcon_mode_deleted(info, &mode1);
			if (!ret)
				fb_delete_videomode(&mode1, &info->modelist);
		}

		return ret ? -EINVAL : 0;
	}

	if (!(var->activate & FB_ACTIVATE_FORCE) &&
	    !memcmp(&info->var, var, sizeof(struct fb_var_screeninfo)))
		return 0;

	activate = var->activate;

	/* When using FOURCC mode, make sure the red, green, blue and
	 * transp fields are set to 0.
	 */
	if ((info->fix.capabilities & FB_CAP_FOURCC) &&
	    var->grayscale > 1) {
		if (var->red.offset     || var->green.offset    ||
		    var->blue.offset    || var->transp.offset   ||
		    var->red.length     || var->green.length    ||
		    var->blue.length    || var->transp.length   ||
		    var->red.msb_right  || var->green.msb_right ||
		    var->blue.msb_right || var->transp.msb_right)
			return -EINVAL;
	}

	if (!info->fbops->fb_check_var) {
		*var = info->var;
		return 0;
	}

	/* bitfill_aligned() assumes that it's at least 8x8 */
	if (var->xres < 8 || var->yres < 8)
		return -EINVAL;

	/* Too huge resolution causes multiplication overflow. */
	if (check_mul_overflow(var->xres, var->yres, &unused) ||
	    check_mul_overflow(var->xres_virtual, var->yres_virtual, &unused))
		return -EINVAL;

	ret = info->fbops->fb_check_var(var, info);

	if (ret)
		return ret;

	if ((var->activate & FB_ACTIVATE_MASK) != FB_ACTIVATE_NOW)
		return 0;

	if (info->fbops->fb_get_caps) {
		ret = fb_check_caps(info, var, activate);

		if (ret)
			return ret;
	}

	old_var = info->var;
	info->var = *var;

	if (info->fbops->fb_set_par) {
		ret = info->fbops->fb_set_par(info);

		if (ret) {
			info->var = old_var;
			printk(KERN_WARNING "detected "
				"fb_set_par error, "
				"error code: %d\n", ret);
			return ret;
		}
	}

	fb_pan_display(info, &info->var);
	fb_set_cmap(&info->cmap, info);
	fb_var_to_videomode(&mode, &info->var);

	if (info->modelist.prev && info->modelist.next &&
	    !list_empty(&info->modelist))
		ret = fb_add_videomode(&mode, &info->modelist);

	if (ret)
		return ret;

	event.info = info;
	event.data = &mode;
	fb_notifier_call_chain(FB_EVENT_MODE_CHANGE, &event);

	return 0;
}
EXPORT_SYMBOL(fb_set_var);

int
fb_blank(struct fb_info *info, int blank)
{
	struct fb_event event;
	int ret = -EINVAL;

	if (blank > FB_BLANK_POWERDOWN)
		blank = FB_BLANK_POWERDOWN;

	event.info = info;
	event.data = &blank;

	if (info->fbops->fb_blank)
		ret = info->fbops->fb_blank(blank, info);

	if (!ret)
		fb_notifier_call_chain(FB_EVENT_BLANK, &event);

	return ret;
}
EXPORT_SYMBOL(fb_blank);

struct class *fb_class;
EXPORT_SYMBOL(fb_class);

static int fb_check_foreignness(struct fb_info *fi)
{
	const bool foreign_endian = fi->flags & FBINFO_FOREIGN_ENDIAN;

	fi->flags &= ~FBINFO_FOREIGN_ENDIAN;

#ifdef __BIG_ENDIAN
	fi->flags |= foreign_endian ? 0 : FBINFO_BE_MATH;
#else
	fi->flags |= foreign_endian ? FBINFO_BE_MATH : 0;
#endif /* __BIG_ENDIAN */

	if (fi->flags & FBINFO_BE_MATH && !fb_be_math(fi)) {
		pr_err("%s: enable CONFIG_FB_BIG_ENDIAN to "
		       "support this framebuffer\n", fi->fix.id);
		return -ENOSYS;
	} else if (!(fi->flags & FBINFO_BE_MATH) && fb_be_math(fi)) {
		pr_err("%s: enable CONFIG_FB_LITTLE_ENDIAN to "
		       "support this framebuffer\n", fi->fix.id);
		return -ENOSYS;
	}

	return 0;
}

static bool apertures_overlap(struct aperture *gen, struct aperture *hw)
{
	/* is the generic aperture base the same as the HW one */
	if (gen->base == hw->base)
		return true;
	/* is the generic aperture base inside the hw base->hw base+size */
	if (gen->base > hw->base && gen->base < hw->base + hw->size)
		return true;
	return false;
}

static bool fb_do_apertures_overlap(struct apertures_struct *gena,
				    struct apertures_struct *hwa)
{
	int i, j;
	if (!hwa || !gena)
		return false;

	for (i = 0; i < hwa->count; ++i) {
		struct aperture *h = &hwa->ranges[i];
		for (j = 0; j < gena->count; ++j) {
			struct aperture *g = &gena->ranges[j];
			printk(KERN_DEBUG "checking generic (%llx %llx) vs hw (%llx %llx)\n",
				(unsigned long long)g->base,
				(unsigned long long)g->size,
				(unsigned long long)h->base,
				(unsigned long long)h->size);
			if (apertures_overlap(g, h))
				return true;
		}
	}

	return false;
}

static void do_unregister_framebuffer(struct fb_info *fb_info);

#define VGA_FB_PHYS 0xA0000
static void do_remove_conflicting_framebuffers(struct apertures_struct *a,
					       const char *name, bool primary)
{
	int i;

	/* check all firmware fbs and kick off if the base addr overlaps */
	for_each_registered_fb(i) {
		struct apertures_struct *gen_aper;

		if (!(registered_fb[i]->flags & FBINFO_MISC_FIRMWARE))
			continue;

		gen_aper = registered_fb[i]->apertures;
		if (fb_do_apertures_overlap(gen_aper, a) ||
			(primary && gen_aper && gen_aper->count &&
			 gen_aper->ranges[0].base == VGA_FB_PHYS)) {

			printk(KERN_INFO "fb%d: switching to %s from %s\n",
			       i, name, registered_fb[i]->fix.id);
			do_unregister_framebuffer(registered_fb[i]);
		}
	}
}

static bool lockless_register_fb;
module_param_named_unsafe(lockless_register_fb, lockless_register_fb, bool, 0400);
MODULE_PARM_DESC(lockless_register_fb,
	"Lockless framebuffer registration for debugging [default=off]");

static int do_register_framebuffer(struct fb_info *fb_info)
{
	int i, ret;
	struct fb_videomode mode;

	if (fb_check_foreignness(fb_info))
		return -ENOSYS;

	do_remove_conflicting_framebuffers(fb_info->apertures,
					   fb_info->fix.id,
					   fb_is_primary_device(fb_info));

	if (num_registered_fb == FB_MAX)
		return -ENXIO;

	num_registered_fb++;
	for (i = 0 ; i < FB_MAX; i++)
		if (!registered_fb[i])
			break;
	fb_info->node = i;
	refcount_set(&fb_info->count, 1);

	fb_chrdev_create(fb_info);

	if (fb_info->pixmap.addr == NULL) {
		fb_info->pixmap.addr = kmalloc(FBPIXMAPSIZE, GFP_KERNEL);
		if (fb_info->pixmap.addr) {
			fb_info->pixmap.size = FBPIXMAPSIZE;
			fb_info->pixmap.buf_align = 1;
			fb_info->pixmap.scan_align = 1;
			fb_info->pixmap.access_align = 32;
			fb_info->pixmap.flags = FB_PIXMAP_DEFAULT;
		}
	}
	fb_info->pixmap.offset = 0;

	if (!fb_info->pixmap.blit_x)
		fb_info->pixmap.blit_x = ~(u32)0;

	if (!fb_info->pixmap.blit_y)
		fb_info->pixmap.blit_y = ~(u32)0;

	if (!fb_info->modelist.prev || !fb_info->modelist.next)
		INIT_LIST_HEAD(&fb_info->modelist);

	if (fb_info->skip_vt_switch)
		pm_vt_switch_required(fb_info->dev, false);
	else
		pm_vt_switch_required(fb_info->dev, true);

	fb_var_to_videomode(&mode, &fb_info->var);
	fb_add_videomode(&mode, &fb_info->modelist);
	registered_fb[i] = fb_info;

#ifdef CONFIG_GUMSTIX_AM200EPD
	{
		struct fb_event event;
		event.info = fb_info;
		fb_notifier_call_chain(FB_EVENT_FB_REGISTERED, &event);
	}
#endif

	if (!lockless_register_fb)
		console_lock();
	else
		atomic_inc(&ignore_console_lock_warning);
	lock_fb_info(fb_info);
	ret = fbcon_fb_registered(fb_info);
	unlock_fb_info(fb_info);

	if (!lockless_register_fb)
		console_unlock();
	else
		atomic_dec(&ignore_console_lock_warning);
	return ret;
}

static void unbind_console(struct fb_info *fb_info)
{
	int i = fb_info->node;

	if (WARN_ON(i < 0 || i >= FB_MAX || registered_fb[i] != fb_info))
		return;

	console_lock();
	lock_fb_info(fb_info);
	fbcon_fb_unbind(fb_info);
	unlock_fb_info(fb_info);
	console_unlock();
}

static void unlink_framebuffer(struct fb_info *fb_info)
{
	int i;

	i = fb_info->node;
	if (WARN_ON(i < 0 || i >= FB_MAX || registered_fb[i] != fb_info))
		return;

	if (!fb_info->dev)
		return;

	fb_chrdev_destroy(fb_info);

	pm_vt_switch_unregister(fb_info->dev);

	unbind_console(fb_info);

	fb_info->dev = NULL;
}

static void do_unregister_framebuffer(struct fb_info *fb_info)
{
	unlink_framebuffer(fb_info);
	if (fb_info->pixmap.addr &&
	    (fb_info->pixmap.flags & FB_PIXMAP_DEFAULT)) {
		kfree(fb_info->pixmap.addr);
		fb_info->pixmap.addr = NULL;
	}

	fb_destroy_modelist(&fb_info->modelist);
	registered_fb[fb_info->node] = NULL;
	num_registered_fb--;
	fb_cleanup_device(fb_info);
#ifdef CONFIG_GUMSTIX_AM200EPD
	{
		struct fb_event event;
		event.info = fb_info;
		fb_notifier_call_chain(FB_EVENT_FB_UNREGISTERED, &event);
	}
#endif
	console_lock();
	fbcon_fb_unregistered(fb_info);
	console_unlock();

	/* this may free fb info */
	put_fb_info(fb_info);
}

/**
 * remove_conflicting_framebuffers - remove firmware-configured framebuffers
 * @a: memory range, users of which are to be removed
 * @name: requesting driver name
 * @primary: also kick vga16fb if present
 *
 * This function removes framebuffer devices (initialized by firmware/bootloader)
 * which use memory range described by @a. If @a is NULL all such devices are
 * removed.
 */
int remove_conflicting_framebuffers(struct apertures_struct *a,
				    const char *name, bool primary)
{
	bool do_free = false;

	if (!a) {
		a = alloc_apertures(1);
		if (!a)
			return -ENOMEM;

		a->ranges[0].base = 0;
		a->ranges[0].size = ~0;
		do_free = true;
	}

	mutex_lock(&registration_lock);
	do_remove_conflicting_framebuffers(a, name, primary);
	mutex_unlock(&registration_lock);

	if (do_free)
		kfree(a);

	return 0;
}
EXPORT_SYMBOL(remove_conflicting_framebuffers);

/**
 * remove_conflicting_pci_framebuffers - remove firmware-configured framebuffers for PCI devices
 * @pdev: PCI device
 * @name: requesting driver name
 *
 * This function removes framebuffer devices (eg. initialized by firmware)
 * using memory range configured for any of @pdev's memory bars.
 *
 * The function assumes that PCI device with shadowed ROM drives a primary
 * display and so kicks out vga16fb.
 */
int remove_conflicting_pci_framebuffers(struct pci_dev *pdev, const char *name)
{
	struct apertures_struct *ap;
	bool primary = false;
	int err, idx, bar;

	for (idx = 0, bar = 0; bar < PCI_STD_NUM_BARS; bar++) {
		if (!(pci_resource_flags(pdev, bar) & IORESOURCE_MEM))
			continue;
		idx++;
	}

	ap = alloc_apertures(idx);
	if (!ap)
		return -ENOMEM;

	for (idx = 0, bar = 0; bar < PCI_STD_NUM_BARS; bar++) {
		if (!(pci_resource_flags(pdev, bar) & IORESOURCE_MEM))
			continue;
		ap->ranges[idx].base = pci_resource_start(pdev, bar);
		ap->ranges[idx].size = pci_resource_len(pdev, bar);
		pci_dbg(pdev, "%s: bar %d: 0x%lx -> 0x%lx\n", __func__, bar,
			(unsigned long)pci_resource_start(pdev, bar),
			(unsigned long)pci_resource_end(pdev, bar));
		idx++;
	}

#ifdef CONFIG_X86
	primary = pdev->resource[PCI_ROM_RESOURCE].flags &
					IORESOURCE_ROM_SHADOW;
#endif
	err = remove_conflicting_framebuffers(ap, name, primary);
	kfree(ap);
	return err;
}
EXPORT_SYMBOL(remove_conflicting_pci_framebuffers);

/**
 *	register_framebuffer - registers a frame buffer device
 *	@fb_info: frame buffer info structure
 *
 *	Registers a frame buffer device @fb_info.
 *
 *	Returns negative errno on error, or zero for success.
 *
 */
int
register_framebuffer(struct fb_info *fb_info)
{
	int ret;

	mutex_lock(&registration_lock);
	ret = do_register_framebuffer(fb_info);
	mutex_unlock(&registration_lock);

	return ret;
}
EXPORT_SYMBOL(register_framebuffer);

/**
 *	unregister_framebuffer - releases a frame buffer device
 *	@fb_info: frame buffer info structure
 *
 *	Unregisters a frame buffer device @fb_info.
 *
 *	Returns negative errno on error, or zero for success.
 *
 *      This function will also notify the framebuffer console
 *      to release the driver.
 *
 *      This is meant to be called within a driver's module_exit()
 *      function. If this is called outside module_exit(), ensure
 *      that the driver implements fb_open() and fb_release() to
 *      check that no processes are using the device.
 */
void
unregister_framebuffer(struct fb_info *fb_info)
{
	mutex_lock(&registration_lock);
	do_unregister_framebuffer(fb_info);
	mutex_unlock(&registration_lock);
}
EXPORT_SYMBOL(unregister_framebuffer);

/**
 *	fb_set_suspend - low level driver signals suspend
 *	@info: framebuffer affected
 *	@state: 0 = resuming, !=0 = suspending
 *
 *	This is meant to be used by low level drivers to
 * 	signal suspend/resume to the core & clients.
 *	It must be called with the console semaphore held
 */
void fb_set_suspend(struct fb_info *info, int state)
{
	WARN_CONSOLE_UNLOCKED();

	if (state) {
		fbcon_suspended(info);
		info->state = FBINFO_STATE_SUSPENDED;
	} else {
		info->state = FBINFO_STATE_RUNNING;
		fbcon_resumed(info);
	}
}
EXPORT_SYMBOL(fb_set_suspend);

/**
 *	fbmem_init - init frame buffer subsystem
 *
 *	Initialize the frame buffer subsystem.
 *
 *	NOTE: This function is _only_ to be called by drivers/char/mem.c.
 *
 */

static int __init
fbmem_init(void)
{
	int ret;

	if (!proc_create_seq("fb", 0, NULL, &proc_fb_seq_ops))
		return -ENOMEM;

	ret = fb_chrdev_register();
	if (ret)
		goto err_chrdev;

	fb_class = class_create(THIS_MODULE, "graphics");
	if (IS_ERR(fb_class)) {
		ret = PTR_ERR(fb_class);
		pr_warn("Unable to create fb class; errno = %d\n", ret);
		fb_class = NULL;
		goto err_class;
	}

	fb_console_init();

	return 0;

err_class:
	fb_chrdev_unregister();
err_chrdev:
	remove_proc_entry("fb", NULL);
	return ret;
}

#ifdef MODULE
module_init(fbmem_init);
static void __exit
fbmem_exit(void)
{
	fb_console_exit();

	remove_proc_entry("fb", NULL);
	class_destroy(fb_class);
	fb_chrdev_unregister();
}

module_exit(fbmem_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Framebuffer base");
#else
subsys_initcall(fbmem_init);
#endif

int fb_new_modelist(struct fb_info *info)
{
	struct fb_var_screeninfo var = info->var;
	struct list_head *pos, *n;
	struct fb_modelist *modelist;
	struct fb_videomode *m, mode;
	int err;

	list_for_each_safe(pos, n, &info->modelist) {
		modelist = list_entry(pos, struct fb_modelist, list);
		m = &modelist->mode;
		fb_videomode_to_var(&var, m);
		var.activate = FB_ACTIVATE_TEST;
		err = fb_set_var(info, &var);
		fb_var_to_videomode(&mode, &var);
		if (err || !fb_mode_is_equal(m, &mode)) {
			list_del(pos);
			kfree(pos);
		}
	}

	if (list_empty(&info->modelist))
		return 1;

	fbcon_new_modelist(info);

	return 0;
}

/**
 * framebuffer_alloc - creates a new frame buffer info structure
 *
 * @size: size of driver private data, can be zero
 * @dev: pointer to the device for this fb, this can be NULL
 *
 * Creates a new frame buffer info structure. Also reserves @size bytes
 * for driver private data (info->par). info->par (if any) will be
 * aligned to sizeof(long).
 *
 * Returns the new structure, or NULL if an error occurred.
 *
 */
struct fb_info *framebuffer_alloc(size_t size, struct device *dev)
{
#define BYTES_PER_LONG (BITS_PER_LONG/8)
#define PADDING (BYTES_PER_LONG - (sizeof(struct fb_info) % BYTES_PER_LONG))
	int fb_info_size = sizeof(struct fb_info);
	struct fb_info *info;
	char *p;

	if (size)
		fb_info_size += PADDING;

	p = kzalloc(fb_info_size + size, GFP_KERNEL);

	if (!p)
		return NULL;

	info = (struct fb_info *) p;

	if (size)
		info->par = p + fb_info_size;

	info->device = dev;
	info->fbcon_rotate_hint = -1;

#if IS_ENABLED(CONFIG_FB_BACKLIGHT)
	mutex_init(&info->bl_curve_mutex);
#endif

	return info;
#undef PADDING
#undef BYTES_PER_LONG
}
EXPORT_SYMBOL(framebuffer_alloc);

/**
 * framebuffer_release - marks the structure available for freeing
 *
 * @info: frame buffer info structure
 *
 * Drop the reference count of the device embedded in the
 * framebuffer info structure.
 *
 */
void framebuffer_release(struct fb_info *info)
{
	if (!info)
		return;
	kfree(info->apertures);
	kfree(info);
}
EXPORT_SYMBOL(framebuffer_release);

#if IS_ENABLED(CONFIG_FB_BACKLIGHT)
/* This function generates a linear backlight curve
 *
 *     0: off
 *   1-7: min
 * 8-127: linear from min to max
 */
void fb_bl_default_curve(struct fb_info *fb_info, u8 off, u8 min, u8 max)
{
	unsigned int i, flat, count, range = (max - min);

	mutex_lock(&fb_info->bl_curve_mutex);

	fb_info->bl_curve[0] = off;

	for (flat = 1; flat < (FB_BACKLIGHT_LEVELS / 16); ++flat)
		fb_info->bl_curve[flat] = min;

	count = FB_BACKLIGHT_LEVELS * 15 / 16;
	for (i = 0; i < count; ++i)
		fb_info->bl_curve[flat + i] = min + (range * (i + 1) / count);

	mutex_unlock(&fb_info->bl_curve_mutex);
}
EXPORT_SYMBOL_GPL(fb_bl_default_curve);
#endif

MODULE_LICENSE("GPL");
