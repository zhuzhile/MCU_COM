#include <reg52.h>
#include "Delay.h"

#define uchar unsigned char

sbit LCD_rs=P2^6;
sbit LCD_rw=P2^5;
sbit LCD_en=P2^7;

void write_command(uchar command){
	LCD_rs=0;
	P0=command;
	delay(5);
	LCD_en=1;
	delay(5);
	LCD_en=0;
}

void write_data(uchar TR_data){
	LCD_rs=1;
	P0=TR_data;
	delay(5);
	LCD_en=1;
	delay(5);
	LCD_en=0;
}

void LCD_init(){
	LCD_rw=0;
	LCD_en=0;
	write_command(0x38);		
	write_command(0x0c);
	write_command(0x06);
	write_command(0x01);		//显示清0，指针清0
}


/** * @brief LCD1602设置光标位置 * @param Line 行位置，范围：1~2 * @param Column 列位置，范围：1~16 * @retval 无 */
void set_cursor(unsigned char Line, unsigned char Column)
{
    
	if(Line==1)
	{
    
		write_command(0x80 | (Column-1));
	}
	else if(Line==2)
	{
    
		write_command(0x80 | (Column-1+0x40));
	}
}



/** * @brief 在LCD1602指定位置上显示一个字符 * @param Line 行位置，范围：1~2 * @param Column 列位置，范围：1~16 * @param Char 要显示的字符 * @retval 无 */
void show_char(unsigned char Line, unsigned char Column, char Char)
{
    
	set_cursor(Line, Column);
	write_data(Char);
}


/** * @brief 在LCD1602指定位置开始显示所给字符串 * @param Line 起始行位置，范围：1~2 * @param Column 起始列位置，范围：1~16 * @param String 要显示的字符串 * @retval 无 */
void show_string(unsigned char Line, unsigned char Column, char *String)
{
    
	unsigned char i;
	set_cursor(Line, Column);
	for(i = 0; String[i] != '\0'; i++)
	{
    
		write_data(String[i]);
	}
}

/** * @brief 返回值=X的Y次方 */
int get_pow(int X, int Y)
{
    
	unsigned char i;
	int Result = 1;
	for(i = 0; i < Y; i++)
	{
    
		Result *= X;
	}
	return Result;
}

/** * @brief 在LCD1602指定位置开始显示所给数字 * @param Line 起始行位置，范围：1~2 * @param Column 起始列位置，范围：1~16 * @param Number 要显示的数字，范围：0~65535 * @param Length 要显示数字的长度，范围：1~5 * @retval 无 */
void show_num(unsigned char Line, unsigned char Column, unsigned int Number, unsigned char Length)
{
    
	unsigned char i;
	set_cursor(Line, Column);
	for(i = Length; i > 0; i--)
	{
    
		write_data(Number/get_pow(10,i-1)%10+'0');
	}
}




