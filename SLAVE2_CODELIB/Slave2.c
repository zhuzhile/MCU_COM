#include <reg52.h>
#include "LCD1602.h"
#include "Delay.h"

#define uchar unsigned char 
#define uint unsigned int
#define OSC 120000000
#define BAUDRATE 9600

uchar R_buf[10],T_buf[10];		//接收数据缓存
uint RPtr, TPtr; // 接收与发送指针
uchar check, len, R_check; // 校验位和数据长度
uchar slave_addr = 0x02; // 本机地址 
uchar R_addr = 0x00;    // 接收到地址
uint time_count = 0; //计时
bit Succ = 1; //判断从机是否接收成功
bit R_done = 0; // 接收完成
bit T_done = 0 ;// 发送完成
bit A_acc = 0; // 判断地址有效
sbit LED = P2^3;
sbit RS485_TR = P3^2;

#define TXEN RS485_TR=1 //发送使能
#define RXEN RS485_TR=0 //接收使能


void serial_init(){
	TMOD = 0X20; // 定时器1选择模式2，8位自动重载
	TL1 = 256 - (OSC / BAUDRATE / 12 / 16); // 硬件自动重载
	TH1 = 256 - (OSC / BAUDRATE / 12 / 16);
	TR1 = 1;     // 开启计数
	SCON = 0xF0; // 串口工作方式3
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


void show_data(TR_data){  // 显示数据
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
	
    RPtr = 0; // 初始化接收
	  R_done = 0;
		RXEN;

		while(!R_done); //接收完成后比对校验位
		R_done = 0;
     
		R_check = R_buf[0];
		for(i = 1; i < len + 3;i++){
			R_check += R_buf[i];
		}
	  Succ = ~Succ; //用于测试
	 // Succ = (R_check == R_buf[len + 3] ? 1 : 0);

		mock_data();  // 根据接收数据，模拟数据
		send_data();  //校验后向主机发送数据
}



void main(){
	LED = 0;
	serial_init();  // 串口初始化
	LCD_init();     // LCD显示初始化

	
	write_command(0x80); // 光标位置
	write_data('R');
	write_data(':');
	while(1){
		RI = 0;
		while(!RI); // 等到触发接收中断时跳出
		
		show_test_data(4, A_acc); //异常测试代码
		
	  if(R_addr == slave_addr && A_acc){ // 确定地址,A_acc 用于模仿超过七毫秒没有接收到数据
			show_test_data(5, A_acc);
			send_addr(slave_addr);
			SM2 = 0;
		}else {
			delay(10); //延迟10ms没有发送数据
			A_acc = ~A_acc;
			continue;
		}				

		recv_data(); // 接收数据，然后发送数据
		
		
		while(!Succ){ // 如果接收失败重新接收
			recv_data();
		}
		
		show_data(R_buf[3]); // 数据接收成功，则显示数据
		
		RPtr = 0;  //接受主机发送终止请求
		R_done = 0;
		RXEN;
		
		while(!R_done); //通信结束
		SM2 = 1;
		RPtr = 0;
		Succ = 0;
	}
}


void comISR() interrupt 4 { //串口通信中断服务
	if(RI == 1){  
		RI = 0;
		if(RB8 == 0){
			delay(1);
		  R_buf[RPtr++] = SBUF;
                 
			show_test_data(1, R_buf[RPtr - 1]); //测试代码
			
		  if( RPtr >= 4 + R_buf[2]){ //主机地址+从机地址+数据长度+数据+校验和
			  
				show_test_data(3, RPtr);  //测试代码
				
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
			
			show_test_data(0, T_buf[TPtr - 1]); //测试代码
			
			if(TPtr >= T_buf[1] + 3){
				
					show_test_data(2, TPtr); //测试代码
					T_done = 1;
					TPtr = 0;
			}
		}	
	}
}



void timer0ISR() interrupt 1{ // 定时器0中断服务timer	
	time_count ++;
	timer0_init();
}