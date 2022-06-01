#include <reg52.h>
#include "Delay.h"

#define uchar unsigned char;
sbit key1=P1^0;
sbit key2=P1^1;
sbit key3=P1^2;
sbit key4=P1^3;


int getKeyNum(void){
	unsigned char k=0;
	
	if(key1==0){
		delay(20);
		while(key1 == 0);
		delay(20);
		k = 1;		
	}
	
	if(key2==0){
		delay(20);
		while(key2 == 0);
		delay(20);
		k = 2;
	}
	
	if(key3==0){
		delay(20);
		while(key3==0);
		delay(20);
		k = 3;
	}
	
	if(key4==0){
		delay(20);
		while(key4 == 0);
		delay(20);
		k = 4;
	}
	
	return k;
}

