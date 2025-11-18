#ifndef LCD_API_H

#define LCD_API_H

/* LCD commands */
#define LCD_CMD_4DL_2N_5X8F		0x28
#define LCD_CMD_DON_CURON		0x0E
#define LCD_CMD_INCADD			0x06
#define LCD_CMD_DIS_CLEAR		0X01
#define LCD_CMD_DIS_RESET		0x02

#define LCD_CMD_SET_CGRAM_ADDRESS	0x40
#define LCD_CMD_SET_DDRAM_ADDRESS	0x80

#define DDRAM_FIRST_LINE_BASE_ADDR	LCD_CMD_SET_DDRAM_ADDRESS
#define DDRAM_SECOND_LINE_BASE_ADDR	(LCD_CMD_SET_DDRAM_ADDRESS | 0x40)

void lcd_enable(struct device *dev);
void lcd_send_command(struct device *dev, uint8_t command);
void lcd_print_char(struct device *dev, uint8_t data);
void lcd_print_string(struct device *dev, char *msg);
void lcd_set_cursor(struct device *dev, u8 row, u8 column);
void lcd_display_reset(struct device *dev);
void lcd_display_clear(struct device *dev);
void lcd_deinit(struct device *dev);
int lcd_init(struct device *dev);

#endif

