/**
  ******************************************************************************
  * @file    Project/STM32_CPAL_Template/stm32_it.c 
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    21-December-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "Includes.h"

extern SEQUEUE RX; 
extern SEQUEUE TX; 

extern u8 rx_enqueue(u8 x);
extern u8 rx_dequeue(void); 
extern u8 tx_enqueue(u8 x);
extern u8 tx_dequeue(void);

extern volatile unsigned  short int  ADC_ConvertedValue[60],ADC_ResultValue[6];

extern u8 WIFISendData[1406];
extern u8 TransmissionData[1332];

u8 startReadWifiCommondData =0;
u8 startReadWifiCommondDataFlag =0;
u8 stopReadWifiCommondDataFlag =0;

u8 startProcessWifiCommondData =0;
u8 startProcessWifiCommondDataFlag =0;
u8 IPHONE_COMMOND_BUF[256];
u8 IPHONE_COMMOND_BUF_INDEX=0; 


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/******************************************************************************/
/*            Cortex-MX Processor Exceptions Handlers (X can be 3 or 4)       */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

void USART1_IRQHandler(void)
{

}

/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : This function handles USART3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_IRQHandler(void)
{
  u8 res;
  if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
      {
        res= USART_ReceiveData(USART3);
        judgeIsIphoneSendCommond(res);

        if((USART3_RX_STA&(1<<15))==0)
        {
            if(USART3_RX_STA<USART3_MAX_RECV_LEN)	//Can also receive data
            {
                TIM7->CNT=0;         			//Counter empty
                if(USART3_RX_STA==0) 			//Enable timer 7 interrupt
                {
                        TIM7->CR1|=1<<0;     		//Enable timer 7
                }
                USART3_RX_BUF[USART3_RX_STA++]=res;	//Record the received value
            }
            else
            {
                USART3_RX_STA|=1<<15;
            }
        }

      }
}

void TIM7_IRQHandler(void)
{ 	  		    
    if(TIM7->SR&0X01)                   //Is an update interrupt
    {
            USART3_RX_STA|=1<<15;	//Tag reception completed
            TIM7->SR&=~(1<<0);		//Clear interrupt flag
            TIM7->CR1&=~(1<<0);		//Turn off timer 7
    }
} 

void TIM4_IRQHandler(void)
{ 
    TIM4->SR&=~(1<<0);		     //Clear interrupt flag
    TIM4->CNT=0;         	     //Counter empty
    WIFIReceivedCommandProcess();    //Receive phone command
}

void TIM3_IRQHandler(void)
{ 
    TIM3->SR&=~(1<<0);		//Clear interrupt flag
    TIM3->CNT=0;         	//Counter empty		
    Process_cmd();	        //Receive USB commands
}
/*******************************************************************************
* Function Name  : DMA1_Channel1_IRQHandler
* Description    : This function handles DMA1 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel1_IRQHandler(void)
{
  ReadVoltage();  //Read ADC Value    	
}

/******************************************************************************/
/*                 STM32xxxx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handlers name please refer to the startup */
/*  file (startup_stm32xxxx_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/*******************************************************************************
* Function Name  : USB_IRQHandler
* Description    : This function handles USB Low Priority interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_Istr();
}

/*******************************************************************************
* Function Name  : USB_HP_CAN1_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN1_TX_IRQHandler(void)
{
	CTR_HP();
}


/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
