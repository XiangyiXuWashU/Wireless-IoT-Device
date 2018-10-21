#ifndef __USART3_H
#define __USART3_H 

#include "stdio.h"	  
//////////////////////////////////////////////////////////////////////////////////	   

//Serial port 3 initialization code

////////////////////////////////////////////////////////////////////////////////// 	

#define USART3_MAX_RECV_LEN		128
#define USART3_MAX_SEND_LEN		1500
#define USART3_RX_EN 			1

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN];
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN];
extern u16 USART3_RX_STA;
void usart3_init(void);

void TIM4_Set(u8 sta);

void TIM3_Set(u8 sta);

void TIM7_Int_Init(u16 arr,u16 psc);

void TIM4_Init(u16 arr,u16 psc);

void TIM3_Init(u16 arr,u16 psc);

void u3_printf(char* fmt, ...);
#endif
















