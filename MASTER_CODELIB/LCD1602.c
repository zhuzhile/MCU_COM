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
	write_command(0x01);		//��ʾ��0��ָ����0
}


/** * @brief LCD1602���ù��λ�� * @param Line ��λ�ã���Χ��1~2 * @param Column ��λ�ã���Χ��1~16 * @retval �� */
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



/** * @brief ��LCD1602ָ��λ������ʾһ���ַ� * @param Line ��λ�ã���Χ��1~2 * @param Column ��λ�ã���Χ��1~16 * @param Char Ҫ��ʾ���ַ� * @retval �� */
void show_char(unsigned char Line, unsigned char Column, char Char)
{
    
	set_cursor(Line, Column);
	write_data(Char);
}


/** * @brief ��LCD1602ָ��λ�ÿ�ʼ��ʾ�����ַ��� * @param Line ��ʼ��λ�ã���Χ��1~2 * @param Column ��ʼ��λ�ã���Χ��1~16 * @param String Ҫ��ʾ���ַ��� * @retval �� */
void show_string(unsigned char Line, unsigned char Column, char *String)
{
    
	unsigned char i;
	set_cursor(Line, Column);
	for(i = 0; String[i] != '\0'; i++)
	{
    
		write_data(String[i]);
	}
}

/** * @brief ����ֵ=X��Y�η� */
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

/** * @brief ��LCD1602ָ��λ�ÿ�ʼ��ʾ�������� * @param Line ��ʼ��λ�ã���Χ��1~2 * @param Column ��ʼ��λ�ã���Χ��1~16 * @param Number Ҫ��ʾ�����֣���Χ��0~65535 * @param Length Ҫ��ʾ���ֵĳ��ȣ���Χ��1~5 * @retval �� */
void show_num(unsigned char Line, unsigned char Column, unsigned int Number, unsigned char Length)
{
    
	unsigned char i;
	set_cursor(Line, Column);
	for(i = Length; i > 0; i--)
	{
    
		write_data(Number/get_pow(10,i-1)%10+'0');
	}
}




