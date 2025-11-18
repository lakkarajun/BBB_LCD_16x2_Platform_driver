/* SPDX-License-Identifier: GPL-2.0 */
/*
 * lcd16x2.h - HD44780 compatible 16x2 LCD Platform Driver
 *
 * Author: Nagaraju Lakkaraju 
 * Date: 18th Nov 2025 
 *
 * Description:
 *   Header file for a BeagleBone Black Linux kernel platform driver
 *   controlling a character LCD (16x2) using GPIO pins.
 *
 * Notes:
 *   - This header defines data structures, macros, and function
 *     prototypes used by lcd16x2.c
 *   - The LCD is assumed to use 4-bit communication mode.
 */

#include <linux/delay.h>
#include "lcd_platform_driver.h"
#include "lcd_gpio.h"
#include "lcd_api.h"

void lcd_enable(struct device *dev)
{
	lcd_gpio_set_value(dev, LCD_EN, LOW_VALUE);
	udelay(10);

	lcd_gpio_set_value(dev, LCD_EN, HIGH_VALUE);
	udelay(10);

	lcd_gpio_set_value(dev, LCD_EN, LOW_VALUE);
	udelay(100); /* execution time > 37 micro seconds */
}

static void write_4_bits(struct device *dev, uint8_t data)
{
	/* 4 bits parallel data write */
	lcd_gpio_set_value(dev, LCD_D4, ((data >> 0 ) & 0x1));
	lcd_gpio_set_value(dev, LCD_D5, ((data >> 1 ) & 0x1));
	lcd_gpio_set_value(dev, LCD_D6, ((data >> 2 ) & 0x1));
	lcd_gpio_set_value(dev, LCD_D7, ((data >> 3 ) & 0x1));

	lcd_enable(dev);
}

void lcd_send_command(struct device *dev, uint8_t command)
{
//	dev_info(dev, "command: 0x%02X (%d)\n", command, command);
	/* RS = 0 for LCD command */
	lcd_gpio_set_value(dev, LCD_RS, LOW_VALUE);

	/*R/nW = 0, for write */
	lcd_gpio_set_value(dev, LCD_RW, LOW_VALUE);

	write_4_bits(dev, (command >> 4)); /* higher nibble */
	write_4_bits(dev, command);        /* lower nibble */
}

/*
 * This function sends a character to the LCD 
 * Here we used 4 bit parallel data transmission. 
 * First higher nibble of the data will be sent on to the data lines D4,D5,D6,D7
 * Then lower niblle of the data will be set on to the data lines D4,D5,D6,D7
 */
void lcd_print_char(struct device *dev, uint8_t data)
{
	/* RS = 1, for user data */
	lcd_gpio_set_value(dev, LCD_RS, HIGH_VALUE);

	/* R/nW = 0, for write */
	lcd_gpio_set_value(dev, LCD_RW, LOW_VALUE);

	write_4_bits(dev, (data >> 4));	/* higher nibble */
	write_4_bits(dev, data);        /* lower nibble */
}

void lcd_print_string(struct device *dev, char *msg)
{
//	dev_info(dev, "msg: %s\n", msg);
	do {
		lcd_print_char(dev, (u8)*msg++);
	} while (*msg != '\0');
}

/**
  * @brief  Set Lcd to a specified location given by row and column information
  * @param  Row Number (1 to 2)
  * @param  Column Number (1 to 16) Assuming a 2 X 16 characters display
  */
void lcd_set_cursor(struct device *dev, u8 row, u8 column)
{
	column--;

	switch (row) {
	case 1:
      		/* Set cursor to 1st row address and add index*/
      		lcd_send_command(dev, (column |= DDRAM_FIRST_LINE_BASE_ADDR));
		break;
    	case 2:
      		/* Set cursor to 2nd row address and add index*/
        	lcd_send_command(dev, (column |= DDRAM_SECOND_LINE_BASE_ADDR));
		break;
	default:
		break;
	}
}

void lcd_display_reset(struct device *dev)
{
	lcd_send_command(dev, LCD_CMD_DIS_RESET);
	/*
	 * check page number 24 of datasheet.
	 * return home command execution wait time is around 2ms
	 */
	mdelay(2);
}

void lcd_display_clear(struct device *dev)
{
	lcd_send_command(dev, LCD_CMD_DIS_CLEAR);
	/*
	 * check page number 24 of datasheet.
	 * display clear command execution wait time is around 2ms
	 */
	mdelay(2);
}

void lcd_deinit(struct device *dev)
{
	lcd_display_clear(dev);
	lcd_display_reset(dev);
}

int lcd_init(struct device *dev)
{
	char init_str_1[] = "16x2 LCD Driver";
	char init_str_2[] = "Developed by Raju";

	lcd_gpio_set_dir(dev, LCD_RS, GPIO_DIR_OUT);
	lcd_gpio_set_dir(dev, LCD_RW, GPIO_DIR_OUT);
	lcd_gpio_set_dir(dev, LCD_EN, GPIO_DIR_OUT);
	lcd_gpio_set_dir(dev, LCD_D4, GPIO_DIR_OUT);
	lcd_gpio_set_dir(dev, LCD_D5, GPIO_DIR_OUT);
	lcd_gpio_set_dir(dev, LCD_D6, GPIO_DIR_OUT);
	lcd_gpio_set_dir(dev, LCD_D7, GPIO_DIR_OUT);

        lcd_gpio_set_value(dev, LCD_RS, GPIO_LOW_VALUE);
        lcd_gpio_set_value(dev, LCD_EN, GPIO_LOW_VALUE);

        lcd_gpio_set_value(dev, LCD_D4, GPIO_LOW_VALUE);
        lcd_gpio_set_value(dev, LCD_D5, GPIO_LOW_VALUE);
        lcd_gpio_set_value(dev, LCD_D6, GPIO_LOW_VALUE);
        lcd_gpio_set_value(dev, LCD_D7, GPIO_LOW_VALUE);

        mdelay(40);

        /* RS = 0 for LCD command */
        lcd_gpio_set_value(dev, LCD_RS, LOW_VALUE);

        /* R/nW = 0, for write */
        lcd_gpio_set_value(dev, LCD_RW, LOW_VALUE);

	write_4_bits(dev, 0x03);
	mdelay(5);

	write_4_bits(dev, 0x03);
	udelay(150);

	write_4_bits(dev, 0x03);
	write_4_bits(dev, 0x02);

	/* 4 bit data mode, 2 lines selection , font size 5x8 */
	lcd_send_command(dev, LCD_CMD_4DL_2N_5X8F);

	/* Display ON, Cursor ON */
	lcd_send_command(dev, LCD_CMD_DON_CURON);

	lcd_display_clear(dev);

	/* Address auto increment */
	lcd_send_command(dev, LCD_CMD_INCADD);

	/* Display init string */
	lcd_print_string(dev, &init_str_1[0]);
	lcd_set_cursor(dev, 2, 1);
	lcd_print_string(dev, &init_str_2[0]);

//	dev_info(dev, "LCD initialization successful\n");
	return 0;
}

