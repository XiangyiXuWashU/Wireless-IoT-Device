#include "Includes.h"

//////////////////////////////////////////////////////////////////////////////////	  
//Serial port 3 initialization
////////////////////////////////////////////////////////////////////////////////// 	

//Serial port send buffer
__align(8) u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; 	//Transmit buffer, max USART3_MAX_SEND_LEN bytes
//Serial receive buffer
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 	                //Receive buffer, maximum USART3_MAX_RECV_LEN bytes.

// Determine whether it is a continuous data by judging that the time difference between receiving two consecutive characters is not more than 10ms.
//If the 2-character reception interval exceeds 10ms, it is considered not to be 1 consecutive data. That is, it has not been received for more than 10ms.
// Any data indicates that the reception is complete.
// Received data status
//[15]:0, no data was received; 1. A batch of data was received.
//[14:0]: the length of the received data

u16 USART3_RX_STA=0;   	 
  
//³õÊ¼»¯IO ´®¿Ú3
void usart3_init(void)
{  	 
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  /* USART3_TX -> PB10 , USART3_RX ->PB11*/				
  /* Configure USART3 Tx (PB.10) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
  /* Configure USART3 Rx (PB.11)  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  USART_InitStructure.USART_BaudRate = 115200;
  USART_Init(USART3, &USART_InitStructure);//Configure USART 
	
  USART3->CR1|=1<<3;  			//Serial port transmission enable
  USART3->CR1|=1<<2;  			//Serial Receive Enable
  USART3->CR1|=1<<5;    		//Receive buffer non-null interrupt enable
  USART3->CR1|=1<<13;  			//Serial port enable

  TIM7_Int_Init(99,719);	        //10ms interrupt once
  TIM7->CR1&=~(1<<0);		        //Turn off timer 7
  USART3_RX_STA=0;			//clear
}
// serial port 3, printf function
// Make sure that the data sent at one time does not exceed USART3_MAX_SEND_LEN bytes
void u3_printf(char* fmt,...)  
{  
    u16 i,j;
    va_list ap;
    va_start(ap,fmt);
    vsprintf((char*)USART3_TX_BUF,fmt,ap);
    va_end(ap);
    i=strlen((const char*)USART3_TX_BUF);//The length of the data sent this time
    for(j=0;j<i;j++)//Loop data
    {
        while((USART3->SR&0X40)==0);//Loop through until the send is complete
        USART3->DR=USART3_TX_BUF[j];
    }
}

// General timer 7 interrupt initialization
// The clock selection is 2 times that of APB1, and APB1 is 36M.
// arr: Automatically reload values.
// psc: clock prescaler
// Timer overflow time calculation method: Tout = ((arr + 1) * (psc + 1) / Ft us.
// Ft=Timer working frequency, unit: Mhz
void TIM7_Int_Init(u16 arr,u16 psc)
{
    RCC->APB1ENR|=1<<5;//TIM7 clock enable
    TIM7->ARR=arr;     //Set counter auto reload value
    TIM7->PSC=psc;     //prescaler
    TIM7->CNT=0;       //Counter clear
    TIM7->DIER|=1<<0;  //Allow update interrupt
    TIM7->CR1|=0x01;   //Enable timer 7

} 
		   
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Set the switch of TIM4
// sta:0, off; 1, on;
void TIM4_Set(u8 sta)
{
   if(sta)
    {
      TIM4->CNT=0;         //Counter empty
      TIM4->CR1|=1<<0;     //Enable timer 4
    }
   else
      TIM4->CR1&=~(1<<0);     //Turn off timer 4
}

// General purpose timer interrupt initialization
// This is always 2 times the APB1, and APB1 is 36M.
// arr: Automatically reload values.
// psc: clock prescaler
void TIM4_Init(u16 arr,u16 psc)
{
    RCC->APB1ENR|=1<<2;	 //TIM4 clock enable
    TIM4->ARR=arr;  	 //Set counter auto reload value
    TIM4->PSC=psc;  	 //prescaler
    TIM4->CNT=0;  	 //Counter cleared
    TIM4->DIER|=1<<0;    //Allow update interrupt
    TIM4->CR1|=0x01;  	 //Enable timer 4
}		   

// Set the switch of TIM3
// sta:0, off; 1, on;
void TIM3_Set(u8 sta)
{
    if(sta)
    {
        TIM3->CNT=0;         //Counter cleared
        TIM3->CR1|=1<<0;     //Enable timer 4
    }
    else
        TIM3->CR1&=~(1<<0); //Turn off timer 4
}

// General purpose timer interrupt initialization
// This is always 2 times the APB1, and APB1 is 36M.
// arr: Automatically reload values.
// psc: clock prescaler
void TIM3_Init(u16 arr,u16 psc)
{
    RCC->APB1ENR|=1<<2;	 //TIM4 clock enable
    TIM3->ARR=arr;  	 //et counter auto reload value
    TIM3->PSC=psc;  	 //prescaler
    TIM3->CNT=0;  		 //Counter cleared
    TIM3->DIER|=1<<0;        //Allow update interrupt
    TIM3->CR1|=0x01;  	 //Enable timer 4
}	



























