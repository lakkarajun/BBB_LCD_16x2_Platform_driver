/* SPDX-License-Identifier: GPL-2.0 */
/*
 * lcd_gpio.c/h – GPIO Interface Header for 16x2 LCD (HD44780)
 *
 * Author: Nagaraju Lakkaraju
 * Date: 18th Nov 2025
 *
 * Description:
 *    This header provides:
 *      • GPIO signal definitions
 *      • Data structure for LCD GPIOs
 *      • Prototypes for GPIO-level send functions
 *
 * Notes:
 *    This file exposes ONLY GPIO-related APIs.
 */

#include <linux/gpio/consumer.h>
#include <linux/device.h>

#include "lcd_platform_driver.h"
#include "lcd_gpio.h"

/* GPIO configuration direction dir: 1 means 'out', 0 means 'in' */
int lcd_gpio_set_dir(struct device *dev, u8 desc_id, u8 dir)
{
	struct lcd_private_data *lcd_data = dev_get_drvdata(dev);
	int status;

	if (dir)
		status = gpiod_direction_output(lcd_data->desc[desc_id],0);
	else
		status = gpiod_direction_input(lcd_data->desc[desc_id]);

	return status;
}

/* GPIO set value value: can be either 0 or 1 */
int lcd_gpio_set_value(struct device *dev, u8 desc_id, u8 value)
{
	struct lcd_private_data *lcd_data = dev_get_drvdata(dev);

	gpiod_set_value(lcd_data->desc[desc_id], value);

	return 0;
}

