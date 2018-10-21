#include "Includes.h"

extern double Temperature;
extern double BiasCurrent;	
extern double ModulateCurrent;

extern u8  FlashData[8];          //used to store data to be stored in Flash

extern u8 WIFISendData[1406];     //All data sent to the phone
extern u8 TransmissionData[1332]; //Spectrum data sent to the phone
extern int DisplayData[333];


// Set the NVIC grouping
// NVIC_Group: NVIC group 0~4 A total of 5 groups
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
    u32 temp,temp1;
    temp1=(~NVIC_Group)&0x07;//take the last three digits
    temp1<<=8;
    temp=SCB->AIRCR;  //Read previous settings
    temp&=0X0000F8FF; //Clear the previous grouping
    temp|=0X05FA0000; //write key
    temp|=temp1;
    SCB->AIRCR=temp;  //Set grouping
}

//Set NVIC
//NVIC_PreemptionPriority:Preemption priority
//NVIC_SubPriority       :Response priority
//NVIC_Channel           :Interrupt number
//NVIC_Group             :Interrupt group 0~4
//Note that the priority cannot exceed the range of the set group! Otherwise there will be unexpected errors.
//Group division:
// Group 0: 0 bit preemption priority, 4 bit response priority
// Group 1: 1 bit preemption priority, 3 bit response priority
// Group 2: 2 bit preemption priority, 2 bit response priority
// Group 3: 3 bit preemption priority, 1 bit response priority
// Group 4: 4 bit preemption priority, 0 bit response priority
//NVIC_SubPriority and NVIC_PreemptionPriority: The principle is that the smaller the value, the higher the priority
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 
{ 
    u32 temp;
    MY_NVIC_PriorityGroupConfig(NVIC_Group);        //Set the grouping
    temp=NVIC_PreemptionPriority<<(4-NVIC_Group);
    temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
    temp&=0xf;					//Take the lower four
    NVIC->ISER[NVIC_Channel/32]|=1<<NVIC_Channel%32;//Enable the interrupt bit (to clear, set the ICER bit to 1)
    NVIC->IP[NVIC_Channel]|=temp<<4;		//Set response priority and steal priority
}

/****************************************************************************
* Name: void RCC_Configuration(void)
* Function: System clock configuration is 72MHZ, peripheral clock configuration
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/ 
ErrorStatus HSEStartUpStatus;

void RCC_Configuration(void)
{
  RCC_DeInit();	   // RCC system reset(for debug purpose)
  RCC_HSEConfig(RCC_HSE_ON); //Enable HSE  
  HSEStartUpStatus = RCC_WaitForHSEStartUp();// Wait till HSE is ready

  if(HSEStartUpStatus == SUCCESS)
  {		      
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);// Enable Prefetch Buffer    
    FLASH_SetLatency(FLASH_Latency_2);  //Flash 2 wait state
      
    RCC_HCLKConfig(RCC_SYSCLK_Div1);    //HCLK = SYSCLK
    RCC_PCLK2Config(RCC_HCLK_Div1);     //PCLK2 = HCLK
    RCC_PCLK1Config(RCC_HCLK_Div2);	//PCLK1 = HCLK/2
    
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // PLLCLK = 8MHz * 9 = 72 MHz	      
    RCC_PLLCmd(ENABLE);	// Enable PLL

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
   }

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                        RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  /* DMA2 clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
  /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
  /* TIM2 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  /* TIM7 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
  /* TIM4 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  /* TIM3 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
}


/****************************************************************************
* Name: void NVIC_Configuration(void)
* Function: Interrupt source configuration function
* Entry parameters: none
* Export parameters: none
* Description:
* Call method:
****************************************************************************/ 
void NVIC_Configuration(void)
{
    MY_NVIC_Init(1,2,DMA1_Channel1_IRQn,0);//ADC Interrupt Priority Configuration
    MY_NVIC_Init(0,0,USART3_IRQn,0);//Group 0, priority 0, 0, highest priority
    MY_NVIC_Init(0,1,TIM7_IRQn,0);	//Preemption 0, sub-priority 1, group 0
    MY_NVIC_Init(2,1,TIM4_IRQn,0);	//Preemption 2, sub-priority 1, group 0
    MY_NVIC_Init(3,1,TIM3_IRQn,0);	//Preempt 3, sub-priority 1, group 0
}

void SwitchLaser(int State)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    if(State==1)
    {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        GPIO_ResetBits(GPIOB, GPIO_Pin_7);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        GPIO_SetBits(GPIOD, GPIO_Pin_13);
    }

    else
     {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        GPIO_SetBits(GPIOB, GPIO_Pin_7);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        GPIO_ResetBits(GPIOD, GPIO_Pin_13);
     }
}

void SwitchTEC(int State)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    if(State==1)
    {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        GPIO_ResetBits(GPIOD, GPIO_Pin_6);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        GPIO_SetBits(GPIOD, GPIO_Pin_12);
    }

  else
     {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        GPIO_SetBits(GPIOD, GPIO_Pin_6);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        GPIO_ResetBits(GPIOD, GPIO_Pin_12);
     }
}


void SwitchDATALED(int State)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    if(State==1)
    {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        GPIO_SetBits(GPIOD, GPIO_Pin_15);
    }

   else
     {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        GPIO_ResetBits(GPIOD, GPIO_Pin_15);
     }
}

void InitEXTRALED(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOD, GPIO_Pin_14);
}

void FlashEXTRALED(void)
{
    GPIO_SetBits(GPIOD, GPIO_Pin_14);
    GPIO_ResetBits(GPIOD, GPIO_Pin_14);
}

double LimitRangeValue(double input, double min, double max, double defaultValue, int isUseDefaultOutrange)
{	 
    if(input<min)
    {
         if(isUseDefaultOutrange)
                 input=defaultValue;
         else
                 input= min;
    }
    else if(input>max)
    {
         if(isUseDefaultOutrange)
                 input=defaultValue;
         else
                 input= max;
    }
    else
    {
         input=input;
    }

    return input;
}
/****************************************************************************
****************************************************************************/  
void InitMainBoard(void)
{
  RCC_Configuration();       //System clock initialization and port peripheral clock enable
  NVIC_Configuration();	     //Interrupt source configuration

  usart3_init();             //Serial 3 configuration

  Synchronize_ADC_DAC();     //ADC and DAC sync
  InitData();                //initialization data
		
  SwitchTEC(0);              //Turn off TEC
  SwitchLaser(0);            //Turn off Laser
  InitEXTRALED();            //Init EXTRALED
	
  ReadFlashData(0x0807FFF8,6);
	
  Temperature = (float)(FlashData[0]+(FlashData[1]<<8))/100;
  BiasCurrent = (float)(FlashData[2]+(FlashData[3]<<8))/100;
  ModulateCurrent = (float)(FlashData[4]+(FlashData[5]<<8))/100;
	
  Temperature = LimitRangeValue(Temperature, 0, 50, 25,1);
  BiasCurrent = LimitRangeValue(BiasCurrent, 0, 180, 125,1);	
  ModulateCurrent = LimitRangeValue(ModulateCurrent, 0, 40, 0,1);
	
  SetTemperature(Temperature);  //Set Temperature
	SetCurrent(BiasCurrent,ModulateCurrent); //Set Current

  InitUSB();               //USB initialization
  delay_ms(600);

  atk_8266_UDP_Init();     //WIFI module initialization
	
  TIM4_Init(99,719);       //1ms interrupt
  TIM4_Set(1);		   //Turn on the TIM4 timer

  TIM3_Init(1999,719);	   //20ms interrupt
  TIM3_Set(1);	           //Turn on the TIM3 timer
  InitIWDG();	          //watchdog initialization
}		



	

		







