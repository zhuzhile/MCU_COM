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