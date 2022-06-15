#include <reg52.h>
#include <stdlib.h>
#include "KeyScan.h"
#include "Delay.h"
#include "LCD1602.h"

#define uint unsigned int
#define uchar unsigned char

sbit LED=P2^3;
sbit RS485_TR=P3^2;

#define TXEN RS485_TR=1; //����ʹ��
#define RXEN RS485_TR=0; //����ʹ��
#define OSC 120000000
#define BAUDRATE 9600


bit R_done = 0;  // �жϽ����Ƿ����
bit T_done = 0;  // �жϷ����Ƿ����
bit Succ = 1;    // �жϽ��������Ƿ���Ч�����ݳ���Ϊ0����Ч��

uint TPtr,RPtr; // �����뷢��ָ��
uint len,check; // ���ݳ��ȣ�У����
uint time_count = 0; //�ӳ�ʱ�����
uchar R_buf[10];	//�������ݻ���
uchar T_buf[10]; //�������ݻ���
uchar H_addr = 0x00; //������ַ
uchar T_addr= 0x01; //���͵�ַ
uchar T_data= 0x00; //��������
uchar R_addr = 0x00; //���մӻ���ַ
uchar send_addr_counter = 0; //���͵�ַ����

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
uint create_stop_check(); // ����ֹͣУ��λ

void main(){
	LED = 0;
	serial_init();
	LCD_init();
	
	T_addr = 0x01; //Ĭ�Ϸ��͵�ַΪ�ӻ�1
	T_data = 0;
	
	while(1){
		show_headline();
		handle_key();
	}
}


void mock_data(){ // ģ�ⷢ������
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
		  show_string(2, 9, "_T_"); //�������ݱ�־
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
		case 1:
			show_string(2, 9, "_R_"); //�������ݱ�־
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
		case 2:
			show_string(2, 9, "TD_"); //����������ɱ�־
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
		case 3:
			show_string(2, 9, "RD_"); //����������ɱ�־
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
		case 4:
			show_string(2, 9, "RA_"); //���յ�ַ��־
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
  		break;
		case 5:
			show_string(2, 9, "TA_"); //���͵�ַ��־
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
		case 6:
			show_string(2, 9, "RTA"); //���·��͵�ַ��־
			delay(1000);
			show_num(2, 9, TR_data, 3);
			delay(1000);
			break;
		case 7:
			show_string(2, 9, "RT_"); //���·������ݱ�־
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
	show_test_data(index, T_addr); // ���Դ���
	
	time_count = 0;
	timer0_init();
	while(time_count < 7); //�ȴ�7ms����
	
	
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
		
		RXEN;           //��ʼ��������
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
			
			TB8 = 0; // ��ʼ��������
			SM2 = 0;
			mock_data();	
			send_data();   
			
			while(!Succ){       // ������յ������д������·���
				send_data();
			}
			
			show_endline(T_addr, 2); // ������յ�����û�����⣬��ʾ���͵�����
			send_stop_signal(); // ��ʼ���ͳ���Ϊ0���ݣ���ʾ������ֹ
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
	TMOD = 0x20; // ��ʱ��1������ģʽ2
	TL1 = 256 - (OSC / BAUDRATE / 12 / 16); // Ӳ���Զ�����
	TH1 = 256 - (OSC / BAUDRATE / 12 / 16);
	PCON |= 0X80;
	TR1 = 1; // ��������
	SCON = 0xD8; // ����3��TB8Ϊ1��RENΪ1 
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


void comISR()interrupt 4{ // ͨ�Ŵ����жϷ���
	
	if( TI == 1 && TB8 == 0){
		uchar index;
		TI = 0;
		
		if(!T_done){
			TB8 = 0;
			SBUF = T_buf[TPtr++];
			
			index = Succ ? 0 : 7;
			
  		show_test_data(index, T_buf[TPtr - 1]); //���Դ���
			
			if(TPtr >=  T_buf[2] + 4){
				RXEN;
				show_test_data(2, TPtr); //���Դ���
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
			
			show_test_data(1, R_buf[RPtr - 1]); //���Դ���

		  if( RPtr >= 3 + R_buf[1]){ //�ӻ���ַ+���ݳ���+����+У���
				
 	  			show_test_data(3, RPtr); //���Դ���
					
				  Succ = (R_buf[1] == 0 ? 0 : 1);
					R_done = 1;
		  }
		}
		
		if(RB8 == 1){
	      R_addr = SBUF;
		}
	}
}


void timer0ISR() interrupt 1{ // ��ʱ��0�жϷ���timer	
	time_count ++;
	if(time_count < 7){
		TH0 = (65535 - 1000)/256;
		TL0 = (65535 - 1000)%256;
	}
}

