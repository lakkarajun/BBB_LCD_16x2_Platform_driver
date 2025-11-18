#ifndef LCD_PLATFORM_DRIVER_H

#define LCD_PLATFORM_DRIVER_H

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>

enum {
	LCD_RS,
	LCD_RW,
	LCD_EN,
	LCD_D4,
	LCD_D5,
	LCD_D6,
	LCD_D7,
	LCD_GPIO_COUNT
};

/* LCD device private data strucuture */
struct lcd_private_data {
	int lcd_scroll;
	char lcdxy[8];  /* Cursor position - (2, 16) */
	struct gpio_desc *desc[LCD_GPIO_COUNT];
	struct device *sysfs_dev;
};

#endif
