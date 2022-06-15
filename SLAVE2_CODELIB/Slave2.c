#include <reg52.h>
#include "LCD1602.h"
#include "Delay.h"

#define uchar unsigned char 
#define uint unsigned int
#define OSC 120000000
#define BAUDRATE 9600

uchar R_buf[10],T_buf[10];		//�������ݻ���
uint RPtr, TPtr; // �����뷢��ָ��
uchar check, len, R_check; // У��λ�����ݳ���
uchar slave_addr = 0x02; // ������ַ 
uchar R_addr = 0x00;    // ���յ���ַ
uint time_count = 0; //��ʱ
bit Succ = 1; //�жϴӻ��Ƿ���ճɹ�
bit R_done = 0; // �������
bit T_done = 0 ;// �������
bit A_acc = 0; // �жϵ�ַ��Ч
sbit LED = P2^3;
sbit RS485_TR = P3^2;

#define TXEN RS485_TR=1 //����ʹ��
#define RXEN RS485_TR=0 //����ʹ��


void serial_init(){
	TMOD = 0X20; // ��ʱ��1ѡ��ģʽ2��8λ�Զ�����
	TL1 = 256 - (OSC / BAUDRATE / 12 / 16); // Ӳ���Զ�����
	TH1 = 256 - (OSC / BAUDRATE / 12 / 16);
	TR1 = 1;     // ��������
	SCON = 0xF0; // ���ڹ�����ʽ3
	PCON |= 0x80;
	EA = 1;
	ES = 1;
	RXEN;
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

void show_test_data(uchar index,uchar TR_data){
	
	switch(index){
		case 0:
		  show_string(2,1, "_T_");
			delay(1000);
			show_num(2, 1, TR_data, 3);
			delay(1000);
			break;
		case 1:
			show_string(2,1, "_R_");
			delay(1000);
			show_num(2, 1, TR_data, 3);
			delay(1000);
			break;
		case 2:
			show_string(2, 1, "TD_");
			delay(1000);
			show_num(2, 1, TR_data, 3);
			delay(1000);
			break;
		case 3:
			show_string(2, 1, "RD_");
			delay(1000);
			show_num(2, 1, TR_data, 3);
			delay(1000);
			break;
		case 4:
			show_string(2, 1, "RA_");
			delay(1000);
			show_num(2, 1, TR_data, 3);
			delay(1000);
  		break;
		case 5:
			show_string(2, 1, "TA_");
			delay(1000);
			show_num(2, 1, TR_data, 3);
			delay(1000);
			break;
	}
}


void show_data(TR_data){  // ��ʾ����
	show_num(1, 3, TR_data, 3);
}

void mock_data(){
	uint i = 0,len_data = R_buf[2] + 3;
	
	for( i = 0; i < len_data;i ++){
		if(!Succ && i == 1){
			T_buf[i] = 0;
			break;
		}
		T_buf[i] = R_buf[i + 1];
	}
	
	check = T_buf[0];
	len_data = T_buf[1] == 0 ? 2: T_buf[1] + 2;
	
	for( i = 1 ; i < len_data; i++){
		check += T_buf[i];
	}
	
	T_buf[i] = check;
}



void send_addr(uchar slave_addr){
	 TI = 0;
	 TB8 = 1;
	 TXEN;
	 SBUF = slave_addr;
	 while(!TI);
	 TI=0;
}



void send_data(){
	TI = 0;
	T_done = 0;
	TXEN;
	TPtr = 0;
	TB8 = 0;
	TI = 1;
	while(!T_done)
	T_done = 0; 
}



void recv_data(){
		uint i;
	
    RPtr = 0; // ��ʼ������
	  R_done = 0;
		RXEN;

		while(!R_done); //������ɺ�ȶ�У��λ
		R_done = 0;
     
		R_check = R_buf[0];
		for(i = 1; i < len + 3;i++){
			R_check += R_buf[i];
		}
	  Succ = ~Succ; //���ڲ���
	 // Succ = (R_check == R_buf[len + 3] ? 1 : 0);

		mock_data();  // ���ݽ������ݣ�ģ������
		send_data();  //У�����������������
}



void main(){
	LED = 0;
	serial_init();  // ���ڳ�ʼ��
	LCD_init();     // LCD��ʾ��ʼ��

	
	write_command(0x80); // ���λ��
	write_data('R');
	write_data(':');
	while(1){
		RI = 0;
		while(!RI); // �ȵ����������ж�ʱ����
		
		show_test_data(4, A_acc); //�쳣���Դ���
		
	  if(R_addr == slave_addr && A_acc){ // ȷ����ַ,A_acc ����ģ�³����ߺ���û�н��յ�����
			show_test_data(5, A_acc);
			send_addr(slave_addr);
			SM2 = 0;
		}else {
			delay(10); //�ӳ�10msû�з�������
			A_acc = ~A_acc;
			continue;
		}				

		recv_data(); // �������ݣ�Ȼ��������
		
		
		while(!Succ){ // �������ʧ�����½���
			recv_data();
		}
		
		show_data(R_buf[3]); // ���ݽ��ճɹ�������ʾ����
		
		RPtr = 0;  //��������������ֹ����
		R_done = 0;
		RXEN;
		
		while(!R_done); //ͨ�Ž���
		SM2 = 1;
		RPtr = 0;
		Succ = 0;
	}
}


void comISR() interrupt 4 { //����ͨ���жϷ���
	if(RI == 1){  
		RI = 0;
		if(RB8 == 0){
			delay(1);
		  R_buf[RPtr++] = SBUF;
                 
			show_test_data(1, R_buf[RPtr - 1]); //���Դ���
			
		  if( RPtr >= 4 + R_buf[2]){ //������ַ+�ӻ���ַ+���ݳ���+����+У���
			  
				show_test_data(3, RPtr);  //���Դ���
				
				R_done = 1;
				len = R_buf[2];
		  }
		}
		
		if(RB8 == 1){
	      R_addr = SBUF;
		}
	} 
	
	
	if( TI == 1 && TB8 == 0){
		TI = 0;
		
		if(!T_done){
			TB8 = 0;
			SBUF = T_buf[TPtr++];
			
			show_test_data(0, T_buf[TPtr - 1]); //���Դ���
			
			if(TPtr >= T_buf[1] + 3){
				
					show_test_data(2, TPtr); //���Դ���
					T_done = 1;
					TPtr = 0;
			}
		}	
	}
}



void timer0ISR() interrupt 1{ // ��ʱ��0�жϷ���timer	
	time_count ++;
	timer0_init();
}