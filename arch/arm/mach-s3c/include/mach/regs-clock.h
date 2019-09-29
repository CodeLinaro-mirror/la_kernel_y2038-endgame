#ifdef CONFIG_ARCH_S3C24XX
#define S3C2410_CLKREG(x) ((x) + S3C24XX_VA_CLKPWR)
#define S3C2410_UPLLCON	    S3C2410_CLKREG(0x08)
#define S3C2410_CLKCON	    S3C2410_CLKREG(0x0C)

#define S3C2410_CLKCON_IDLE	     (1<<2)
#define S3C2410_CLKCON_POWER	     (1<<3)
#define S3C2410_CLKCON_NAND	     (1<<4)
#define S3C2410_CLKCON_LCDC	     (1<<5)
#define S3C2410_CLKCON_USBH	     (1<<6)
#define S3C2410_CLKCON_USBD	     (1<<7)
#define S3C2410_CLKCON_PWMT	     (1<<8)
#define S3C2410_CLKCON_SDI	     (1<<9)
#define S3C2410_CLKCON_UART0	     (1<<10)
#define S3C2410_CLKCON_UART1	     (1<<11)
#define S3C2410_CLKCON_UART2	     (1<<12)
#define S3C2410_CLKCON_GPIO	     (1<<13)
#define S3C2410_CLKCON_RTC	     (1<<14)
#define S3C2410_CLKCON_ADC	     (1<<15)
#define S3C2410_CLKCON_IIC	     (1<<16)
#define S3C2410_CLKCON_IIS	     (1<<17)
#define S3C2410_CLKCON_SPI	     (1<<18)
#endif

#ifdef CONFIG_ARCH_S3C64XX
#include "regs-clock-s3c64xx.h"
#endif
