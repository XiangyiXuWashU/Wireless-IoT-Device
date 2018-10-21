#ifndef _DATA_POLL_H
#define _DATA_POLL_H

#define  BUFF_MAX  100

typedef struct{
        unsigned short	     front;
        unsigned short	     rear;
        unsigned char	     uart_buff[BUFF_MAX];
}SEQUEUE;

#define  SLAVE_HEAD1      0x20
#define  SLAVE_HEAD2      0X21
#define  SLAVE_HEAD3      0x22

#define RECV_HEAD_MAX  2 	//define the max head length of SEQUEUE
#define RECV_DATA_MAX  2000	//define the max data length of SEQUEUE
#define UART_BUFFERSIZE 2000	//define the max buffer size of serial interface

typedef struct
{
    unsigned char  Rhd[RECV_HEAD_MAX];
    unsigned char  Rda[RECV_DATA_MAX];
    unsigned char  Rend[2];
}Recv;

void USART_Send_Begin(u8* buff,u16 count);
void InitData(void);
void RecvDepackage(Recv *ComFrame,unsigned char Rbuffer[]);
void Process_cmd(void);
void Data_Poll(unsigned char Check_ok);
void WIFIReceivedCommandProcess(void);
void judgeIsIphoneSendCommond(u8 inputData);

double CalVS(int ADCValue);
double CalVMCU(int ADCValue);
double CalCurrent(int ADCValue);
double CalTemp(int ADCValue);
double CalVLD(int ADCValue);

double mWtoDBM(double PmW);
u8 *ConvertIntArrayToString(int *inputdata,int number,u8 *str);

#endif /*_DATA_POLL_H*/
