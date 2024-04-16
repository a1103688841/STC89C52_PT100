#include <STC89C5xRC.H>
#include <uart.h>


sbit I2C_SCL=P1^6;//�൱�ھ��񣬸ߵ�ƽʱд��/�������͵�ƽʱSDA���иı�
sbit I2C_SDA=P1^5;//�ߵ͵�ƽ����2��������
 
/**
  * @brief  I2C��ʼ
  * @param  ��
  * @retval ��
  */
void I2C_Start(void)
{
	I2C_SDA=1;//SDA����Ϊ1Ҳ����Ϊ0������Ϊ������SCL�͵�ƽ��ʱ��任����(SCL�ڴ��������Ϊ0)�������Ƚ�SDA��1
	I2C_SCL=1;
	I2C_SDA=0;//����ƴͼ��ʼʱSDA����0��SDL��0
	I2C_SCL=0;
}
 
/**
  * @brief  I2Cֹͣ
  * @param  ��
  * @retval ��
  */
void I2C_Stop(void)
{
	I2C_SDA=0;//��ֹͣǰ������RA��0(�յ�����Ӧ��)Ҳ������SA��1(�������ݲ�Ӧ��)�����Խ�SDA��1
	I2C_SCL=1;
	I2C_SDA=1;//����ƴͼ��ʼʱSCL����1��SDA��1
}
 
/**
  * @brief  I2C����һ���ֽ�
  * @param  Byte Ҫ���͵��ֽ�
  * @retval ��
  */
void I2C_SendByte(unsigned char Byte)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		I2C_SDA=Byte&(0x80>>i);//��SCLΪ0��ʱ��任����
		I2C_SCL=1;//��оƬ�ֲ��֪�������ʱС�ڵ�Ƭ��ִ��һ������ʱ�䣬�����м䲻�ü���ʱ
		I2C_SCL=0;
	}
}
 
/**
  * @brief  I2C����һ���ֽ�
  * @param  ��
  * @retval ���յ���һ���ֽ�����
  */
unsigned char I2C_ReceiveByte(void)
{
	unsigned char i,Byte=0x00;//�ֲ��������ֶ�����
	I2C_SDA=1;//����ʱSDA����Ҫ�ͷţ�����1
	for(i=0;i<8;i++)
	{
		I2C_SCL=1;
		if(I2C_SDA){Byte|=(0x80>>i);}
		I2C_SCL=0;
	}
	return Byte;
}
 
/**
  * @brief  I2C����Ӧ��
  * @param  AckBit Ӧ��λ��0ΪӦ��1Ϊ��Ӧ��
  * @retval ��
  */
void I2C_SendAck(unsigned char AckBit)
{
	I2C_SDA=AckBit;
	I2C_SCL=1;
	I2C_SCL=0;
}
 
/**
  * @brief  I2C����Ӧ��λ
  * @param  ��
  * @retval ���յ���Ӧ��λ��0ΪӦ��1Ϊ��Ӧ��
  */
unsigned char I2C_ReceiveAck(void)
{
	unsigned char AckBit;
	I2C_SDA=1;//����ʱSDA����Ҫ�ͷţ�����1
	I2C_SCL=1;
	AckBit=I2C_SDA;
	I2C_SCL=0;
	return AckBit;
}

//���ڴ���
void uart_service()
{
	if(f_slow_down%Uart_Slow) return;//���ڼ��ٳ���
	if(uart_n>0)
	{
		relay=~relay;
	}
	uart_n=0;
}

//T0��ʼ��
void Timer0_Init(void)		//1����@12.000MHz
{
	AUXR &= 0x7F;			//��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0;			//���ö�ʱ��ģʽ
	TL0 = 0x18;				//���ö�ʱ��ʼֵ
	TH0 = 0xFC;				//���ö�ʱ��ʼֵ
	TF0 = 0;				//���TF0��־
	TR0 = 1;				//��ʱ��0��ʼ��ʱ
}


//T0�жϷ���
void T0_service() interrupt 1
{
	if(++f_slow_down==500) f_slow_down=0;

}

//�궨������
#define Uart_Slow 200 //���ڼ���ʱ��
#define Uart_Max_Commad 10 //����ָ�������

//��������
unsigned int f_slow_down;//��������ר�ñ���
unsigned char uart_n;//���ڽ���������������
unsigned char uart_rec[10];//���ڽ�������
unsigned char uart_str[10];//���ڷ�������

//�����жϷ���
void uart1_service() interrupt 4 
{
	if(RI==1&&uart_n<Uart_Max_Commad)//�����յ�����
	{
		uart_rec[uart_n]=SBUF;//������������
		uart_n++;//����ֵ��1
		RI=0;//RI����
		
	}
}
void UartInit(void)		//9600bps@24.000MHz
{
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x01;		//����1ѡ��ʱ��2Ϊ�����ʷ�����
	AUXR &= 0xFB;		//��ʱ��2ʱ��ΪFosc/12,��12T
	TL2 = 0xCC;		//�趨��ʱ��ֵ
	TH2 = 0xFF;		//�趨��ʱ��ֵ
	AUXR |= 0x10;		//������ʱ��2
	ES=1;
}

void send_byte(unsigned char dat)//����һ���ֽ�
{
	SBUF=dat;//��Ҫ���͵����ݸ�ֵ��SBUF�Ĵ���
	while(TI==0);//�����ͳɹ���TI=1��ʱ������ѭ��
	TI=0;//TI����
}

void send_str(unsigned char* str)//����һ���ַ���
{
	while(*str!='\0')//���ַ�û������ʱ
		send_byte(*str++);//��λ�����ַ�
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