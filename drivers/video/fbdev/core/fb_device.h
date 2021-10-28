
int fb_chrdev_register(void);
void fb_chrdev_unregister(void);

struct fb_info;
void fb_chrdev_create(struct fb_info *fb_info);
void fb_chrdev_destroy(struct fb_info *fb_info);

struct fb_info *get_fb_info(unsigned int idx);
void put_fb_info(struct fb_info *fb_info);
