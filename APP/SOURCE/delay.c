#include "Includes.h"
//////////////////////////////////////////////////////////////////////////////////

//delay nus
//nus is the number of us to delay.
void delay_us(u32 nus)
{		
   u32 temp = 0;
   temp = nus;
   SysTick->CTRL &= (u32)0XFFFFFFFB;
   temp = 9 * temp;

   SysTick->LOAD = temp;      
   SysTick->VAL=0x00;   
   SysTick->CTRL=0x01;     
   
   temp = 0;
   do 
   { 
     temp=SysTick->CTRL; 
   } 
   while(temp&0x01&&!(temp&(1<<16)));   
   
   SysTick->CTRL=0x00;       
   SysTick->VAL =0X00; 	
}

//dely nms
void delay_ms(u16 nms)
{	 		  	  
   u32 temp = 0;
   temp = nms;
   SysTick->CTRL &= (u32)0XFFFFFFFB;
   temp = 9000 * temp;

   SysTick->LOAD = temp;      
   SysTick->VAL=0x00;   
   SysTick->CTRL=0x01;     
   
   temp = 0;
   do 
   { 
     temp=SysTick->CTRL; 
   } 
   while(temp&0x01&&!(temp&(1<<16)));   
   
   SysTick->CTRL=0x00;       
   SysTick->VAL =0X00;   	    
} 




























