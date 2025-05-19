#include "reg52.h"	
#include "temp.h"

typedef unsigned int u16;	
typedef unsigned char u8;

sbit LSA = P2^2;
sbit LSB = P2^3;
sbit LSC = P2^4;

sbit KEY_PAUSE = P3^0;
sbit KEY_RESUME = P3^1;
sbit KEY_CLEAR = P3^2;

u8 code smgduan[17] = {
    0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,
    0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71
};

u8 ssec, sec, min;
u8 DisplayData[8]; // ��һ����

void delay(u16 i) { while(i--); }

void Timer0Init() {
    TMOD |= 0x01;
    TH0 = 0xD8;
    TL0 = 0xF0;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void DigDisplay() {
	u8 i;
    for( i=0; i<8; i++) {
        switch(i) {
            case 0: LSA=0; LSB=0; LSC=0; break;
            case 1: LSA=1; LSB=0; LSC=0; break;
            case 2: LSA=0; LSB=1; LSC=0; break;
            case 3: LSA=1; LSB=1; LSC=0; break;
            case 4: LSA=0; LSB=0; LSC=1; break;
            case 5: LSA=1; LSB=0; LSC=1; break;
            case 6: LSA=0; LSB=1; LSC=1; break;
            case 7: LSA=1; LSB=1; LSC=1; break;
        }
        P0 = DisplayData[i];
        delay(50); // ������ʱ
        P0 = 0x00;
    }
}

void datapros(int temp) {
    float current_temp = temp * 0.0625; // ת���¶�
    
    if(current_temp < 30) {
        DisplayData[0] = smgduan[min/10];
        DisplayData[1] = smgduan[min%10];
        DisplayData[2] = 0x40;
        DisplayData[3] = smgduan[sec/10];
        DisplayData[4] = smgduan[sec%10];
        DisplayData[5] = 0x40;
        DisplayData[6] = smgduan[ssec/10];
        DisplayData[7] = smgduan[ssec%10];
    } else {
		u16 display_val = current_temp * 100 + 0.5; // ������λС��
        DisplayData[0] = 0x39;                      // ��ʾ"C"
        DisplayData[1] = smgduan[display_val/1000 % 10];
        DisplayData[2] = smgduan[display_val/100 % 10];
        DisplayData[3] = smgduan[display_val/10 % 10] | 0x80;
        DisplayData[4] = smgduan[display_val % 10];
        DisplayData[5] = 0x00; // �ر�δ����ʾλ
        DisplayData[6] = 0x00;
        DisplayData[7] = 0x00;
 
    }
}

void KeyScan() {
    static bit key_flag = 0;  // ������־λ
    
    if((!KEY_PAUSE || !KEY_RESUME || !KEY_CLEAR) && !key_flag) {
        delay(1000);  // ����ʱ����
        key_flag = 1;
        
        if(!KEY_PAUSE) {      // ��ͣ����
            TR0 = 0;         // ֹͣ��ʱ��
        }
        else if(!KEY_RESUME) { // ��������
            TR0 = 1;         // ������ʱ��
        }
        else if(!KEY_CLEAR) {  // ���㰴��
            TR0 = 0;         // ֹͣ��ʱ��
            min = 0;         // ʱ������
            sec = 0;
            ssec = 0;
        }
    }
    else if(KEY_PAUSE && KEY_RESUME && KEY_CLEAR) {
        key_flag = 0;
    }
}

void main() {
    Timer0Init();
    while(1) {
        datapros(Ds18b20ReadTemp());
        DigDisplay();
        KeyScan();
    }
}

void Timer0() interrupt 1 {
    TH0 = 0xD8;
    TL0 = 0xF0;
    
    if(TR0 && ++ssec >= 100) {
        ssec = 0;
        if(++sec >= 60) {
            sec = 0;
            if(++min >= 60) min = 0;
        }
    }
}