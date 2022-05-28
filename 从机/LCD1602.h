#ifndef __LCD1602_H__
#define __LCD1602_H__

void LCD_init(void);
void write_command(unsigned char command);
void write_data(unsigned char TR_data);

#endif