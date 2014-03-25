#ifndef __EP93XX_PLATFORM_H
#define __EP93XX_PLATFORM_H

#define EP93XX_CHIP_REV_D0	3
#define EP93XX_CHIP_REV_D1	4
#define EP93XX_CHIP_REV_E0	5
#define EP93XX_CHIP_REV_E1	6
#define EP93XX_CHIP_REV_E2	7

unsigned int ep93xx_chip_revision(void);

struct platform_device;

int ep93xx_pwm_acquire_gpio(struct platform_device *pdev);
void ep93xx_pwm_release_gpio(struct platform_device *pdev);
int ep93xx_ide_acquire_gpio(struct platform_device *pdev);
void ep93xx_ide_release_gpio(struct platform_device *pdev);
int ep93xx_keypad_acquire_gpio(struct platform_device *pdev);
void ep93xx_keypad_release_gpio(struct platform_device *pdev);

int ep93xx_i2s_acquire(void);
void ep93xx_i2s_release(void);

#endif
