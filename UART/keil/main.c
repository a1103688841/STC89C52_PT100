#include <STC89C5xRC.H>
#include <uart.h>


sbit I2C_SCL=P1^6;//相当于晶振，高电平时写入/读出，低电平时SDA进行改变
sbit I2C_SDA=P1^5;//高低电平传输2进制数据
 
/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
void I2C_Start(void)
{
	I2C_SDA=1;//SDA可能为1也可能为0，又因为我们在SCL低电平的时候变换数据(SCL在传输结束后都为0)，所以先将SDA置1
	I2C_SCL=1;
	I2C_SDA=0;//按照拼图开始时SDA先置0后SDL置0
	I2C_SCL=0;
}
 
/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
void I2C_Stop(void)
{
	I2C_SDA=0;//在停止前可能是RA：0(收到数据应答)也可能是SA：1(发送数据不应答)，所以将SDA置1
	I2C_SCL=1;
	I2C_SDA=1;//按照拼图开始时SCL先置1后SDA置1
}
 
/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的字节
  * @retval 无
  */
void I2C_SendByte(unsigned char Byte)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		I2C_SDA=Byte&(0x80>>i);//在SCL为0的时候变换数据
		I2C_SCL=1;//查芯片手册可知，最大延时小于单片机执行一条语句的时间，所以中间不用加延时
		I2C_SCL=0;
	}
}
 
/**
  * @brief  I2C接收一个字节
  * @param  无
  * @retval 接收到的一个字节数据
  */
unsigned char I2C_ReceiveByte(void)
{
	unsigned char i,Byte=0x00;//局部变量，手动定义
	I2C_SDA=1;//接收时SDA首先要释放，即置1
	for(i=0;i<8;i++)
	{
		I2C_SCL=1;
		if(I2C_SDA){Byte|=(0x80>>i);}
		I2C_SCL=0;
	}
	return Byte;
}
 
/**
  * @brief  I2C发送应答
  * @param  AckBit 应答位，0为应答，1为非应答
  * @retval 无
  */
void I2C_SendAck(unsigned char AckBit)
{
	I2C_SDA=AckBit;
	I2C_SCL=1;
	I2C_SCL=0;
}
 
/**
  * @brief  I2C接收应答位
  * @param  无
  * @retval 接收到的应答位，0为应答，1为非应答
  */
unsigned char I2C_ReceiveAck(void)
{
	unsigned char AckBit;
	I2C_SDA=1;//接收时SDA首先要释放，即置1
	I2C_SCL=1;
	AckBit=I2C_SDA;
	I2C_SCL=0;
	return AckBit;
}

//串口处理
void uart_service()
{
	if(f_slow_down%Uart_Slow) return;//串口减速程序
	if(uart_n>0)
	{
		relay=~relay;
	}
	uart_n=0;
}

//T0初始化
void Timer0_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x18;				//设置定时初始值
	TH0 = 0xFC;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
}


//T0中断服务
void T0_service() interrupt 1
{
	if(++f_slow_down==500) f_slow_down=0;

}

//宏定义声明
#define Uart_Slow 200 //串口减速时间
#define Uart_Max_Commad 10 //串口指令长度上限

//变量声明
unsigned int f_slow_down;//函数减速专用变量
unsigned char uart_n;//串口接收数组数据索引
unsigned char uart_rec[10];//串口接收数组
unsigned char uart_str[10];//串口发送数组

//串口中断服务
void uart1_service() interrupt 4 
{
	if(RI==1&&uart_n<Uart_Max_Commad)//若接收到数据
	{
		uart_rec[uart_n]=SBUF;//赋给接受数组
		uart_n++;//索引值增1
		RI=0;//RI清零
		
	}
}
void UartInit(void)		//9600bps@24.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR &= 0xFB;		//定时器2时钟为Fosc/12,即12T
	TL2 = 0xCC;		//设定定时初值
	TH2 = 0xFF;		//设定定时初值
	AUXR |= 0x10;		//启动定时器2
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
sbit relay=P1^7;


void main()
{
	Timer0Init();
	UartInit();
	EA=1;
	send_str("seventeen\r\n");
	while(1)
	{
		uart_service();
		//send_str(uart_n);
	}
}