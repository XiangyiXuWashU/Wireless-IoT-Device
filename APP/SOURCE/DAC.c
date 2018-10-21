#include "Includes.h"

#define DAC_DHR12RD_Address      0x40007420

u32 TriangleArray[1000];
										
double Temperature=26.0;	
double BiasCurrent=145;	
double ModulateCurrent=40;

int FlashTemperature;    //FlashTemperature=Temperature*100
int FlashBiasCurrent;    //FlashBiasCurrent=BiasCurrent*100
int FlashModulateCurrent;//FlashModulateCurrent=FlashModulateCurrent*100

const unsigned int Resistance[23] =
      {
        55786,  //address:0x80 = -10C
        42582,  //address:0x82 = -5C
        32773,  //address:0x84 = 0C
        25456,  //address:0x86 = 5C
        19931,  //address:0x88 = 10C
        15725 , //address:0x8A = 15C
        12497 , //address:0x8C = 20C
        10000 , //address:0x8E = 25C
        8055 ,  //address:0x90 = 30C
        6528 ,  //address:0x92 = 35C
        5323 ,  //address:0x94 = 40C
        4365 ,  //address:0x96 = 45C
        3599 ,  //address:0x98 = 50C
        2983 ,  //address:0x9A = 55C
        2486 ,  //address:0x9C = 60C
        2082 ,  //address:0x9E = 65C
        1753 ,  //address:0xA0 = 70C
        1482,   //address:0xA2 = 75C
        1258 ,  //address:0xA4 = 80C
        1073 ,  //address:0xA6 = 85C
        919 ,   //address:0xA8 = 90C
        790 ,   //address:0xAA = 95C
        682 ,   //address:0xAC = 100C
      };

void InitDAC(void) 
{ 
  DAC_InitTypeDef            DAC_InitStructure;
  DMA_InitTypeDef            DMA_InitStructure;

  GPIO_DAC_Configuration();
	
  //  TIM2_Configuration();

  /* DAC channel1 Configuration */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO; //Set laser current using timer trigger
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
	
  DAC_Init(DAC_Channel_1, &DAC_InitStructure); //Chanenel_1 is for setting laser current

  /* DAC channel2 Configuration */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_Software; //Set laser temperatur using software trigger
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;

  DAC_Init(DAC_Channel_2, &DAC_InitStructure); //Chanenel_2 is for setting laser temperature

  GenerateTriangle(BiasCurrent,ModulateCurrent);//Set the default waveform for Channel_1  Inital Current=30; Modulte=0

  /* DMA2 channel3 configuration */
  DMA_DeInit(DMA2_Channel3);

  DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR12RD_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&TriangleArray;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = 1000;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

  DMA_Init(DMA2_Channel3, &DMA_InitStructure);
  /* Enable DMA2 Channel3 */
  DMA_Cmd(DMA2_Channel3, ENABLE);

  /* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is 
     automatically connected to the DAC converter. */
  DAC_Cmd(DAC_Channel_1, ENABLE);
			
  /* Enable DAC Channel2: Once the DAC channel2 is enabled, PA.05 is 
     automatically connected to the DAC converter. */
  DAC_Cmd(DAC_Channel_2, ENABLE);		
	
  /* Enable DMA for DAC Channel1 */
  DAC_DMACmd(DAC_Channel_1, ENABLE);
} 


void GPIO_DAC_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Once the DAC channel is enabled, the corresponding GPIO pin is automatically 
     connected to the DAC converter. */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4|GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  //GPIO_Mode_Out_PP
  GPIO_Init(GPIOA, &GPIO_InitStructure);	
}

void TIM2_Configuration(void)
{
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
  TIM_OCInitTypeDef          TIM_OCInitStructure;
	
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Period = 0x1C1;     //72000000/(TIM_Prescaler+1)/(TIM_Period+1)=10000 10000 cycles/ps
  TIM_TimeBaseStructure.TIM_Prescaler = 0xF;    //converting time for each DAC point is 0.0001s¡£Period=0.0001 s *1000 ints=0.1=10HZ
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
  /* TIM2 TRGO selection */
  TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);

  /* TIM1 channel1 configuration in PWM mode */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;                
  TIM_OCInitStructure.TIM_Pulse =0X7F; 
  TIM_OCInitStructure.TIM_OCPolarity =TIM_OCPolarity_Low;         
  TIM_OC2Init(TIM2, &TIM_OCInitStructure); 	
}

void SetTemperature(float Temp)
{
    u16 DACData=0;
    int index=0;
    float k=0,b=0,CalRes=0,SetVolt=0;

    if(Temp<10)
    {
        Temp=10;
    }
    else if(Temp>50)
    {
        Temp=50;
    }
    else
    {
        Temp=Temp;
    }

    index=(int)(Temp/5)+2;

    k=(Resistance[index]-Resistance[index+1])/5;
    b= Resistance[index]*(index-1)-(Resistance[index+1]*(index-2));
    CalRes=b-(k*Temp);
    SetVolt=CalRes*0.0001;

    DACData=(SetVolt/3.3)*4095;

    DAC_SetChannel2Data(DAC_Align_12b_R,DACData);
    DAC_SoftwareTriggerCmd(DAC_Channel_2, ENABLE);
}

void SetCurrent(double Bias, double Modulate)
{
    GenerateTriangle(Bias,Modulate);
}

void GenerateTriangle(double offset,double Amplitude)
{
    u16 i=0;
    double start,step;
	
    start=(offset-Amplitude/2)*9.927;
    step=Amplitude*0.00993;

    for(i=0;i<1000;i++)
    {

        if((start+step*i)>1985)  //Laser Current<200mA
            {
              TriangleArray[i]=1985;
            }
        else if((start+step*i)<=0)
            {
              TriangleArray[i]=0;
            }
        else
            {
              TriangleArray[i] = start+step*i;
            }
     }
}






