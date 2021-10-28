
#ifdef CONFIG_FB_DEVICE
int fb_chrdev_register(void);
void fb_chrdev_unregister(void);

struct fb_info;
void fb_chrdev_create(struct fb_info *fb_info);
void fb_chrdev_destroy(struct fb_info *fb_info);
#else
static inline int fb_chrdev_register(void)
{
	return 0;
}
static inline void fb_chrdev_unregister(void)
{
}

struct fb_info;
static inline void fb_chrdev_create(struct fb_info *fb_info)
{
}
static inline void fb_chrdev_destroy(struct fb_info *fb_info)
{
}
#endif

struct fb_info *get_fb_info(unsigned int idx);
void put_fb_info(struct fb_info *fb_info);
