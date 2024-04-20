/*------------------------------------------------------------------*/
/* --- STC MCU Limited ---------------------------------------------*/
/* --- STC89-90xx Series MCU UART (8-bit/9-bit)Demo ----------------*/
/* --- Mobile: (86)13922805190 -------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ---------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966----------------------*/
/* --- Web: www.STCMCU.com -----------------------------------------*/
/* --- Web: www.GXWMCU.com -----------------------------------------*/
/* If you want to use the program or the program referenced in the  */
/* article, please specify in which data and procedures from STC    */
/*------------------------------------------------------------------*/

#include "intrins.h"
#include "reg51.h"
#include <stdio.h>

typedef unsigned char BYTE;
typedef unsigned int WORD;

#define FOSC 12000000L // System frequency
#define BAUD 2400      // UART baudrate

bit busy;

/*----------------------------
UART interrupt service routine
----------------------------*/
void Uart_Isr() interrupt 4
{
  if (RI)
  {
    RI = 0;    // Clear receive interrupt flag
    P0 = SBUF; // P0 show UART data
  }
  if (TI)
  {
    TI = 0;   // Clear transmit interrupt flag
    busy = 0; // Clear transmit busy flag
  }
}

/*----------------------------
Send a byte data to UART
Input: dat (data to be sent)
Output:None
----------------------------*/
void SendData(BYTE dat)
{
  while (busy);        // Wait for the completion of the previous data is sent
  ACC = dat; // Calculate the even parity bit P (PSW.0)
  busy = 1;
  SBUF = ACC; // Send data to UART buffer
}

/*----------------------------
Send a string to UART
Input: s (address of string)
Output:None
----------------------------*/
void SendString(char *s)
{
  while (*s) // Check the end of the string
  {
    SendData(*s++); // Send current char and increment string ptr
  }
}
// ADS1110
sbit SCL = P1 ^ 6; // ADS1110
sbit SDA = P1 ^ 5; // ADS1110
unsigned char adc_hi;
unsigned char adc_lo;
/***************************************************************************/
/*    ����ԭ��: void ads1100_start (void) */
/*    ��    ��: �ṩI2C���߹���ʱ���е���ʼλ��                            */
/***************************************************************************/
static void delay5us(void) // ��� 0us
{
  int i;
  for (i = 0; i < 2; i++)
    ;
}

void I2C_start() // SCL Ϊ�ߵ�ƽ�ڼ䣬SDA�Ӹߵ��ͱ任Ϊ��ʼ�ź�
{

  SDA = 1;
  delay5us();
  SCL = 1; // 11111
  delay5us();
  SDA = 0;
  delay5us();
  SCL = 0;
}

void I2C_stop() // SCLΪ�ߵ�ƽ�ڼ䣬SDA�ӵ͵�ƽ���ߵ�ƽ�ı仯Ϊ�����ź�
{
  SDA = 0;
  delay5us();
  SCL = 0;
  delay5us();
  SCL = 1;
  delay5us();
  SDA = 1;
  delay5us();
}

void write_ack(unsigned char ack) // дACK
{
  SDA = ack;
  SCL = 1;
  SCL = 0;
}

void wait_ack() // �ȴ�ACK
{
  unsigned char i = 0;
  SDA = 1;
  SCL = 1;
  while ((SDA == 1) && (i < 250))
    i++;
  SCL = 0;
}

/*I2Cд��һ���ֽ�*/
void I2C_write_byte(unsigned char dat)
{
  unsigned char i, mask;
  mask = 0x80;
  for (i = 0; i < 8; i++)
  {
    SCL = 0;
    if ((mask & dat) == 0)
      SDA = 0;
    else
      SDA = 1;

    mask >>= 1;
    SCL = 1;
    SCL = 0; //*****
  }
  SCL = 0;
  SDA = 1;
  wait_ack();
}

/*��I2C�ڶ�ȡһ���ֽ�*/
unsigned char I2C_read_byte()
{
  unsigned char i, k;
  SCL = 0;
  SDA = 1;
  for (i = 0; i < 8; i++)
  {
    SCL = 1;
    k = (k << 1) | SDA;
    SCL = 0;
  }

  return k;
}

/*��ADS1110��д*/
void write_ads1110(unsigned char cmd)
{
  I2C_start();
  I2C_write_byte(0X90); // ADS1110��ַ д
  I2C_write_byte(cmd);
  I2C_stop();
}
/*��ADS1110���*/
void read_ads1110()
{
  I2C_start();
  I2C_write_byte(0x91); // ADS1110��ַ ��
  adc_hi = I2C_read_byte();
  write_ack(0);
  adc_lo = I2C_read_byte();
  write_ack(1);
  I2C_stop();
}

/*ADS1110��ȡ��ѹ*/
unsigned int Ads1110_volt(void)
{
  unsigned int temp;
  write_ads1110(
      0x8c); // ����ADS1110 ����ת��ģʽ�����ݴ�������15SPS ����Ϊ1 16λת��
  read_ads1110();
  temp = adc_hi * 255 + adc_lo;
  return temp;
}

const float code Pt100_typeThermoTable[106] =
    {
        18.52, 22.83, 27.10, 31.34, 35.54, 39.72, 43.88, 48.00, 52.11, 56.19,
        60.26, 64.30, 68.33, 72.33, 76.33, 80.31, 84.27, 88.22, 92.16, 96.09,
        100.00, 103.90, 107.79, 111.67, 115.54, 119.40, 123.24, 127.08, 130.90, 134.71,
        138.51, 142.29, 146.07, 149.83, 153.58, 157.33, 161.05, 164.77, 168.48, 172.17,
        175.86, 179.53, 183.19, 186.84, 190.47, 194.10, 197.71, 201.31, 204.90, 208.48,
        212.05, 215.61, 219.15, 222.68, 226.21, 229.72, 233.21, 236.70, 240.18, 243.64,
        247.09, 250.53, 253.96, 257.38, 260.78, 264.18, 267.56, 270.93, 274.29, 277.64,
        280.98, 284.30, 287.62, 290.92, 294.21, 297.49, 300.75, 304.01, 307.25, 310.49,
        313.71, 316.92, 320.12, 323.30, 326.48, 329.64, 332.79, 335.93, 339.06, 342.18,
        345.28, 348.38, 351.46, 354.53, 357.59, 360.64, 363.67, 366.70, 369.71, 372.71,
        375.70, 378.68, 381.65, 384.60, 387.55, 390.48

};

int half_search(const float A[], int n, double key)
{
  int low, high, mid;
  double keytemp;
  keytemp = key;
  low = 0;
  high = n - 1;

  if (keytemp > A[high])
  {
    // Uart_Printf("%s(%d):�۰뷵��%d",__MODULE__,__LINE__,high-1);
    return high;
  }
  else if (keytemp < A[0])
  {
    // Uart_Printf("%s(%d):�۰뷵��%d",__MODULE__,__LINE__,0);
    return 0;
  }

  while (low <= n - 1 && high >= 0)
  {
    mid = (low + high + 1) / 2;
    if (A[mid] <= keytemp && keytemp <= A[mid + 1])
    {
      // Uart_Printf("%s(%d):�۰뷵��%d",__MODULE__,__LINE__,mid);
      return mid;
    }
    if (A[mid] < keytemp)
    {
      low = mid + 1;
    }
    if (A[mid] > keytemp)
    {
      high = mid - 1;
    }
  }
  // Uart_Printf("%s(%d):�۰뷵��%d",__MODULE__,__LINE__,-1);
  return -1;
}

//
char buffer[50];
sbit P17 = P1 ^ 7;
void main()
{
  char test = 0;
  unsigned int adc;
  int indexFind, A1, A2;
  float temperature;
  float oumu_estimation;
  SCON = 0x50;                          // 8-bit variable UART
  TMOD = 0x20;                          // Set Timer1 as 8-bit auto reload mode
  TH1 = TL1 = -(FOSC / 12 / 32 / BAUD); // Set auto-reload vaule
  TR1 = 1;                              // Timer1 start run
  ES = 1;                               // Enable UART interrupt
  EA = 1;                               // Open master interrupt switch

  while (1)
  {
    int i, j;
    for (i = 0; i < 100; i++)
    {
      for (j = 0; j < 100; j++);
    }
    adc = Ads1110_volt();
    oumu_estimation = adc * 0.0031 + 96.362;
    indexFind = half_search(Pt100_typeThermoTable, 106, oumu_estimation);
    // û�ҵ�
    if (indexFind == -1)
    {
      indexFind = 0;
      A1 = 0;
      A2 = 0;
      temperature = A1;
      temperature += -200;
    }
    // ���������ֵ���
    else if (indexFind == 106 - 1)
    {
      A1 = indexFind * 10;
      A2 = indexFind * 10;
      temperature = A1;
      temperature += -200;
    }
    // �������
    else
    {
      A1 = Pt100_typeThermoTable[indexFind];
      A2 = Pt100_typeThermoTable[indexFind + 1];
      temperature = (double)((oumu_estimation - A1) / (A2 - A1) * 10 + indexFind * 10);
      temperature += -200;
    }
    //    sprintf(buffer, "%d,%f,%f", adc,oumu_estimation , temperature);
    // SendString(buffer);
    if (temperature > 40.0)
    {
      sprintf(buffer, "waring!!");
      SendString(buffer);
      P17 = 1;
    }
    else
    {
      sprintf(buffer, "%f��", temperature);
      SendString(buffer);
      P17 = 0;
    }
    //  if( test == 0)
    //  {
    // 	 sprintf(buffer, "test0");
    // 	 SendString(buffer);
    //   test = 1;
    //   P17  = 1;
    //   SCL = 0;
    //   SDA = 1;
    //  }
    //  else{
    // 	   	sprintf(buffer, "test1");
    //  	 SendString(buffer);
    //   test = 0;
    //   P17  = 0;
    //   SCL = 1;
    //   SDA = 0;
    //  }
  }
}
