#include <uart.h>

void UartInit(void)		//9600bps@24.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR &= 0xFB;		//定时器2时钟为Fosc/12,即12T
	TL2 = 0xCC;		//设定定时初值
	TH2 = 0xFF;		//设定定时初值
	AUXR |= 0x10;		//启动定时器2
	EA=1;
	ES=1;
}

void send_byte(unsigned char dat)//发送一个字节
{
	SBUF=dat;//将要发送的数据赋值给SBUF寄存器
	while(TI==0);//当发送成功（TI=1）时，跳出循环
	TI=0;//TI清零
}

void send_str(unsigned char* str)//发送一个字符串
{
	while(*str!='\0')//当字符没发送完时
		send_byte(*str++);//按位发送字符
}