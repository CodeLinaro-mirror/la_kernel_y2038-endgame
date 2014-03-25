#ifndef __ASSEMBLY__

#include <linux/reboot.h>
#include <linux/platform_data/net-ep93xx.h>

#include <mach/platform.h>

struct device;
struct i2c_gpio_platform_data;
struct i2c_board_info;
struct spi_board_info;
struct platform_device;
struct ep93xxfb_mach_info;
struct ep93xx_keypad_platform_data;
struct ep93xx_spi_info;

void ep93xx_map_io(void);
void ep93xx_init_irq(void);

void ep93xx_register_flash(unsigned int width,
			   resource_size_t start, resource_size_t size);

void ep93xx_register_eth(struct ep93xx_eth_data *data, int copy_addr);
void ep93xx_register_i2c(struct i2c_gpio_platform_data *data,
			 struct i2c_board_info *devices, int num);
void ep93xx_register_spi(struct ep93xx_spi_info *info,
			 struct spi_board_info *devices, int num);
void ep93xx_register_fb(struct ep93xxfb_mach_info *data);
void ep93xx_register_pwm(int pwm0, int pwm1);
int ep93xx_pwm_acquire_gpio(struct platform_device *pdev);
void ep93xx_pwm_release_gpio(struct platform_device *pdev);
void ep93xx_register_keypad(struct ep93xx_keypad_platform_data *data);
void ep93xx_register_i2s(void);
void ep93xx_register_ac97(void);
void ep93xx_register_ide(void);

struct device *ep93xx_init_devices(void);
extern void ep93xx_timer_init(void);

void ep93xx_restart(enum reboot_mode, const char *);
void ep93xx_init_late(void);

#ifdef CONFIG_CRUNCH
int crunch_init(void);
#else
static inline int crunch_init(void) { return 0; }
#endif

#endif
