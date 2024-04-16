#include <uart.h>

void UartInit(void)		//9600bps@24.000MHz
{
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x01;		//����1ѡ��ʱ��2Ϊ�����ʷ�����
	AUXR &= 0xFB;		//��ʱ��2ʱ��ΪFosc/12,��12T
	TL2 = 0xCC;		//�趨��ʱ��ֵ
	TH2 = 0xFF;		//�趨��ʱ��ֵ
	AUXR |= 0x10;		//������ʱ��2
	EA=1;
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