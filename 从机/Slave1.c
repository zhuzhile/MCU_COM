#include <reg52.h>
#include "LCD1602.h"
#include "Delay.h"

#define uchar unsigned char 
#define uint unsigned int
#define OSC 12000000
#define BAUDRATE 4800

uchar R_buf[10],T_buf[10];		//�������ݻ���
uint RPtr, TPtr; // �����뷢��ָ��
uchar check, len, R_check; // У��λ�����ݳ���
uchar slave_addr = 0x01; // ������ַ 
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

void show_test_data(TR_data){
	
	write_command(0x80+0x40);
  write_data('D');
  write_data(':');
	write_data('0'+TR_data/100);
  write_data('0'+TR_data/10%10);
  write_data('0'+TR_data%10);
}


void show_data(TR_data){  // ��ʾ����
	write_command(0x80+2);
	write_data('0'+TR_data/100);
  write_data('0'+TR_data/10%10);
  write_data('0'+TR_data%10);
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
	len_data = T_buf[1] == 0 ? 2: T_buf[1] + 3;
	
	for( i = 1 ; i < len_data; i++){
		check ^= T_buf[i];
	}
	
	T_buf[i] = check;
}



void send_addr(uchar slave_addr){
	 TI = 0;
	 TB8 = 1;
	 LED=~LED;
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
			R_check ^= R_buf[i];
		}
	  Succ = ~Succ; //���ڲ���
	 // Succ = (R_check == R_buf[len + 3] ? 1 : 0);
		
	//	timer0_init(); // ����ִ��mock_data,�೤ʱ�䣻���Ա�������100ms
		mock_data();  // ���ݽ������ݣ�ģ������
		
	//	show_test_data(25); //���Դ���
	//	delay(1000);        
	//	show_test_data(time_count);
	//	delay(1000);
		
	  delay(1000); //ͬ������
		send_data();  //У�����������������
}



void main(){
	uint i = 0;
	serial_init();  // ���ڳ�ʼ��
	LCD_init();     // LCD��ʾ��ʼ��
	
	write_command(0x80); // ���λ��
	write_data('R');
	write_data(':');
	while(1){
		RI = 0;
		while(!RI); // �ȵ����������ж�ʱ����
		
		show_test_data(29); //���Դ���
		delay(1000);
		show_test_data(A_acc);
		delay(1000);
		
	  if(R_addr == slave_addr && A_acc){ // ȷ����ַ,A_acc ����ģ�³����ߺ���û�н��յ�����		
			show_test_data(30);
	//		delay(100);
			send_addr(slave_addr);
			SM2 = 0;
		}else {
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
	}
}


void comISR() interrupt 4 { //����ͨ���жϷ���
	if(RI == 1){  
		RI = 0;
		if(RB8 == 0){
			
		  R_buf[RPtr++] = SBUF;
                           
			show_test_data(23); //���Դ���
			delay(1000);
			show_test_data(R_buf[RPtr - 1]);
			delay(1000);
			
		  if( RPtr >= 4 + R_buf[2]){ //������ַ+�ӻ���ַ+���ݳ���+����+У���
			  
				show_test_data(24);  //���Դ���
				delay(1000);
  			show_test_data(RPtr);
				delay(1000);
				
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
			
			show_test_data(21); //���Դ���
			delay(1000);
			show_test_data(T_buf[TPtr - 1]);
			delay(1000);
			
			if(TPtr >= T_buf[1] + 3){
				
				 show_test_data(22); //���Դ���
				 delay(1000);
				 show_test_data(TPtr); 
				 delay(1000);
				
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