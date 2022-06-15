#include <reg52.h>
#include <stdlib.h>
#include "KeyScan.h"
#include "Delay.h"
#include "LCD1602.h"

#define uint unsigned int
#define uchar unsigned char

sbit LED=P2^3;
sbit RS485_TR=P3^2;

#define TXEN RS485_TR=1; //发送使能
#define RXEN RS485_TR=0; //接收使能
#define OSC 120000000
#define BAUDRATE 9600


bit R_done = 0;  // 判断接收是否完成
bit T_done = 0;  // 判断发送是否完成
bit Succ = 1;    // 判断接收数据是否有效（数据长度为0则无效）

uint TPtr,RPtr; // 接收与发送指针
uint len,check; // 数据长度，校验结果
uint time_count = 0; //延迟时间计数
uchar R_buf[10];	//接收数据缓存
uchar T_buf[10]; //发送数据缓存
uchar H_addr = 0x00; //主机地址
uchar T_addr= 0x01; //发送地址
uchar T_data= 0x00; //发送数据
uchar R_addr = 0x00; //接收从机地址
uchar send_addr_counter = 0; //发送地址次数

void mock_data();
void show_data(uchar TR_data);
void send_addr();
void show_endline(char addr, uint index);
void send_data();
void handle_key();
void serial_init();
void show_headline();
void timer0_init();
void show_test_data(uchar index, uchar TR_data);
void send_stop_signal();
uint create_stop_check(); // 创造停止校验位

void main(){
	LED = 0;
	serial_init();
	LCD_init();
	
	T_addr = 0x01; //默认发送地址为从机1
	T_data = 0;
	
	while(1){
		show_headline();
		handle_key();
	}
}


void mock_data(){ // 模拟发送数据
	uint i;
	T_buf[0] = H_addr;
	T_buf[1] = T_addr;
	T_buf[2] = len;
	for(i = 0; i < len;i++){
		T_buf[3 + i] = T_data;
	}
	check = T_buf[0];
	for(i = 1; i < len + 3; i++){
			check += T_buf[i];
	}
	
	T_buf[3 + len] = check;
}

void show_data(uchar TR_data){
		write_data('0'+TR_data/100);
		write_data('0'+TR_data/10%10);
		write_data('0'+TR_data%10);	
}

void show_test_data(uchar index,uchar TR_data){
	switch(index){
		case 0:
		  show_string(2, 9, "_T_"); //发送数据标志
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
		case 1:
			show_string(2, 9, "_R_"); //接收数据标志
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
		case 2:
			show_string(2, 9, "TD_"); //发送数据完成标志
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
		case 3:
			show_string(2, 9, "RD_"); //接收数据完成标志
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
		case 4:
			show_string(2, 9, "RA_"); //接收地址标志
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
  		break;
		case 5:
			show_string(2, 9, "TA_"); //发送地址标志
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
		case 6:
			show_string(2, 9, "RTA"); //重新发送地址标志
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
		case 7:
			show_string(2, 9, "RT_"); //重新发送数据标志
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
	}
}


void send_addr(){	
	uchar index;
	TXEN;
	TI = 0;
	TB8 = 1;
	SBUF = T_addr;
	while(!TI);
	TI = 0;
	RXEN;
	RI = 0;
	index = send_addr_counter == 0 ? 5 : 6;
	show_test_data(index, T_addr); // 测试代码
	
	time_count = 0;
	timer0_init();
	while(time_count < 7); //等待7ms过后
	
	
  if(R_addr != T_addr){
		send_addr_counter ++;
		send_addr();
	}
	
}

void show_endline(uchar addr, uint index){
	  write_command(0x80+0x40);	
	  write_data('R');
	  
	  write_data('0' + addr);
		write_data(':');
	  show_data(R_buf[index]);
}

void send_data(){
		TXEN;
		TI = 0;
		TB8 = 0;
		TPtr = 0;
		TI = 1;
	
		while(!T_done);	
		T_done = 0;
		
		RXEN;           //开始接收数据
		RI = 0;
		RPtr = 0;
	
		while(!R_done);
		R_done = 0;		
}


void handle_key(){
	uchar k = 0;
	k = getKeyNum();
	switch(k)
	{
		case 1:
			T_addr = T_addr != 0x01 ? 0x01 : 0x02;  
			break;
		case 2:
			T_data ++;
			break;
		case 3:
			T_data --;
			break;
		case 4:	
			send_addr();
		
			len = rand() % 5 + 1;
			
			TB8 = 0; // 开始发送数据
			SM2 = 0;
			mock_data();	
			send_data();   
			
			while(!Succ){       // 如果接收的数据有错误，重新发送
				send_data();
			}
			
			show_endline(T_addr, 2); // 如果接收的数据没有问题，显示发送的数据
			send_stop_signal(); // 开始发送长度为0数据，表示发送终止
			while(!T_done);
			T_done = 0;
			R_addr = 0x00;
			TXEN;
			break;
	}
}

uint create_stop_check(){ 
	uint i = 0, stop_check = T_buf[0];
	
	for( i = 1; i < 3; i++){
		stop_check += T_buf[i];
	}
	
	return stop_check;
}

void send_stop_signal(){
	  TXEN;
		TI = 0;
		TB8 = 0;
		T_buf[2] = 0;
	
		T_buf[3] = create_stop_check();
		TI = 1;
		while(!T_done);
}


void serial_init(){
	TMOD = 0x20; // 定时器1，工作模式2
	TL1 = 256 - (OSC / BAUDRATE / 12 / 16); // 硬件自动重载
	TH1 = 256 - (OSC / BAUDRATE / 12 / 16);
	PCON |= 0X80;
	TR1 = 1; // 开启计数
	SCON = 0xD8; // 串口3，TB8为1，REN为1 
	EA = 1;
	ES = 1;
}

void timer0_init(){
	TMOD &= 0xF0;
	TMOD |= 0x01;
	TF0 = 0;
	TR0 = 1; 
	TH0 = (65535 - 1000)/256;
	TL0 = (65535 - 1000)%256;
	ET0 = 1;
	PT0 = 1;
	EA = 1;
}


void show_headline(){
		write_command(0x80);
		write_data('T');
		write_command(0x80+3);
		write_data('A');
		write_data(':');
		write_data('0' + T_addr);	
		write_command(0x80+8);
		write_data('D');
		write_data(':');
		show_data(T_data);
}


void comISR()interrupt 4{ // 通信串口中断服务
	
	if( TI == 1 && TB8 == 0){
		uchar index;
		TI = 0;
		
		if(!T_done){
			TB8 = 0;
			SBUF = T_buf[TPtr++];
			
			index = Succ ? 0 : 7;
			
  		show_test_data(index, T_buf[TPtr - 1]); //测试代码
			
			if(TPtr >=  T_buf[2] + 4){
				RXEN;
				show_test_data(2, TPtr); //测试代码
				T_done = 1;
				
				TPtr = 0;
				Succ = 1;
				send_addr_counter = 0;
			}
		}	
	}
	
	if(RI == 1){  
		RI = 0;
	  if(RB8 == 0){
		  R_buf[RPtr ++] = SBUF;
			
			show_test_data(1, R_buf[RPtr - 1]); //测试代码

		  if( RPtr >= 3 + R_buf[1]){ //从机地址+数据长度+数据+校验和
				
 	  			show_test_data(3, RPtr); //测试代码
					
				  Succ = (R_buf[1] == 0 ? 0 : 1);
					R_done = 1;
		  }
		}
		
		if(RB8 == 1){
	      R_addr = SBUF;
		}
	}
}


void timer0ISR() interrupt 1{ // 定时器0中断服务timer	
	time_count ++;
	if(time_count < 7){
		TH0 = (65535 - 1000)/256;
		TL0 = (65535 - 1000)%256;
	}
}

