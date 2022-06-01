#ifndef __LCD1602_H__
#define __LCD1602_H__

void LCD_init(void);
void write_command(unsigned char command);
void write_data(unsigned char TR_data);
void set_cursor(unsigned char Line, unsigned char Column);
void show_char(unsigned char Line, unsigned char Column, char Char);
void show_string(unsigned char Line, unsigned char Column, char *String);
int get_pow(int X, int Y);
void show_num(unsigned char Line, unsigned char Column, unsigned char Number, unsigned char Length);
#endif