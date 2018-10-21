#include "Includes.h"

volatile unsigned  short int  ADC_ConvertedValue[6000],ADC_ResultValue[6],PD_ADCValue[1000];

extern double Temperature;
extern double BiasCurrent;	
extern double ModulateCurrent;

u8 WIFISendData[1406];
u8 TransmissionData[1332];
int TransmissionValue[333];
char SystemValuesStr[74];

int DisplayData[333] =    {
            159,158,157,157,157,156,156,155,155,154,153,153,
            152,152,151,152,150,151,150,150,149,148,148,148,
            147,146,147,146,146,145,145,145,144,144,144,143,
            143,143,143,143,142,142,142,142,141,141,140,141,
            140,140,140,139,139,138,139,139,138,138,137,138,
            138,138,137,137,138,136,137,136,136,136,137,136,
            136,136,135,135,136,135,135,135,135,134,134,133,
            134,134,133,133,132,133,132,132,130,130,129,130,
            130,129,128,128,128,128,127,127,127,125,126,125,
            124,124,122,122,121,120,120,119,119,118,117,115,
            114,113,113,112,110,110,109,107,104,103,101,99,97,
            94,93,89,87,84,82,80,78,74,72,68,65,61,58,54,50,45,
            41,37,33,30,26,22,21,18,15,14,12,9,7,7,6,5,6,6,5,5,
            6,7,8,9,10,11,12,15,17,19,21,23,25,27,31,32,35,37,
            40,42,44,47,49,51,53,55,57,59,62,64,65,68,69,71,74,
            75,77,79,80,83,84,86,87,88,90,92,93,95,96,99,99,100,
            101,103,104,106,107,108,109,111,111,112,113,115,115,
            117,117,119,119,120,121,122,123,123,124,125,126,126,
            127,127,129,129,129,130,131,132,132,134,133,134,135,
            135,136,137,137,138,138,139,139,140,140,141,140,142,
            142,142,143,143,144,145,145,145,146,147,146,146,147,
            148,149,148,149,149,151,150,150,151,151,151,151,152,
            152,152,153,154,153,154,153,154,154,154,155,155,155,
            156,156,156,156,157,157,156,154,154,154,155,155,155,
            156,156,156,156,157,157,153,
          };
/*******************************************************************************
* Function Name  : ADC_Configuration
* Description    : Configure the ADC
* Input          : None
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/
void ADC_Configuration(void)
{ 
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
	
    /* Enable DMA clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);  //ADC sampling divided by 6 72M/6=12M

    /* Enable ADC1,GPIOA, GPIOC clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOC| RCC_APB2Periph_AFIO , ENABLE);

    /* DMA channel1 configuration ----------------------------------------------*/
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_ConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 6000;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Channel1,DMA_IT_TC, ENABLE);

    DMA_Cmd(DMA1_Channel1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* TIM2 Configuration */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC1
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;        //performed in Scan (multichannels)or Single (one channel) mode.
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//the conversion is performed in Continuous or Single mode
    ADC_InitStructure.ADC_ExternalTrigConv =ADC_ExternalTrigConv_T2_CC2;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 6;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* ADC1 regular channels configuration */
    /* TCONV = 7.5(ADC_SampleTime) + 12.5(ADC_CLK)=20 Time Cycle  20*1/12M=1.67uS*/
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 5, ADC_SampleTime_71Cycles5);  //PC3/GRAPH Y  ADC_SampleTime_55Cycles5
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 3, ADC_SampleTime_7Cycles5);	  //PC0/REAL I
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 4, ADC_SampleTime_7Cycles5);	  //PC2/REAL T
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 6, ADC_SampleTime_7Cycles5);	  //PC4/VLD
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_7Cycles5);	  //PA0/VS
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_7Cycles5);	  //PA1/VMCU

    ADC_DMACmd(ADC1, ENABLE);

    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);

    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);

    while(ADC_GetCalibrationStatus(ADC1));

    ADC_ExternalTrigConvCmd(ADC1,ENABLE);
}

void Synchronize_ADC_DAC(void)
{
    TIM2_Configuration();
    ADC_Configuration();
    InitDAC();
    TIM_Cmd(TIM2, ENABLE);
}


void ReadVoltage()
{
  int AD_value=0;
  int i,j;
				 
  if(DMA_GetITStatus(DMA1_IT_TC1))
    {
     for(i=0;i<6;i++)
       {
        AD_value=0;
        for(j=0;j<1000;j++)
                {
                  AD_value += ADC_ConvertedValue[j*6+i];
                }
        AD_value =  AD_value/1000;
        AD_value =  (AD_value*3300)/4096;
        ADC_ResultValue[i] = AD_value;
       }

     for(j=0;j<1000;j++)
      {
        PD_ADCValue[j]=(ADC_ConvertedValue[j*6+4])*0.806;
      }

     Calculate_LaserProtect_CombineSendValue();       //Calculate_Laser Protect_combine wifi send data

     DMA_ClearITPendingBit(DMA1_IT_GL1);
    }
}


/****************************************************************************
//Calculate transmission data
//Calculate VS、VMCU、I、T、VLD  ADC Value
//Laser Protect
//Combine all the wifi sending data
//SystemValuesStr: LDStatus, TECStatus,Temperature,BiasCurrent,ModulateCurrent,0.00,VS,VMCU,I,T,VLD
//TransmissionData
//Data Structure:
//WIFISendData[0]= LDStatus  0=ON 1=OFF
//WIFISendData[1]= TECStatus 0=ON 1=OFF
//WIFISendData[2-9]= Temperature
//WIFISendData[10-17]= BiasCurrent
//WIFISendData[18-25]= ModulateCurrent
//WIFISendData[26-33]= 0.00
//WIFISendData[34-41]= VS
//WIFISendData[42-49]= VMCU
//WIFISendData[50-57]= I
//WIFISendData[58-65]= T
//WIFISendData[66-73]= VLD
//WIFISendData[74-1405]= TransmissionData

****************************************************************************/
void Calculate_LaserProtect_CombineSendValue(void)
{ 	
    double VS,VMCU,I,T,VLD;
    double LDStatus, TECStatus;
	
    int i,j;

    for (i = 0; i < 333; i++)
    {
        for(j=0;j<3;j++)
        {
         TransmissionValue[i] = TransmissionValue[i]+PD_ADCValue[i*3+j];
        }

        TransmissionValue[i] = TransmissionValue[i]/3;
    }
		
    ConvertIntArrayToString(TransmissionValue,333,TransmissionData);

    LDStatus  = GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_7);//LaserStatus
    TECStatus = GPIO_ReadOutputDataBit(GPIOD, GPIO_Pin_6);//TECStatatus

    VS=CalVS(ADC_ResultValue[0]);
    VMCU=CalVMCU(ADC_ResultValue[1]);
    I=CalCurrent(ADC_ResultValue[2]);
    T=CalTemp(ADC_ResultValue[3]);
    VLD=CalVLD(ADC_ResultValue[5]);

    if(I>190)	{SwitchLaser(0); SwitchTEC(0);} //Protect Laser Current
    if(T>50|T<9){SwitchTEC(0); SwitchLaser(0);} //Protect Laser Temperature
    if(VLD>1.8)	{SwitchTEC(0);SwitchLaser(0);}	//Proetect Laser Voltage

    if(VS<0.001)	{VS=0;}
    if(VMCU<0.001)	{VMCU=0;}
    if(I<0.001)	{I=0;}
    if(VLD<0.001)	{VLD=0;}

    sprintf(SystemValuesStr, "%1.0f%1.0f%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f%8.2f",LDStatus, TECStatus,Temperature,BiasCurrent,ModulateCurrent,0.00,VS,VMCU,I,T,VLD);
    sprintf((char*)WIFISendData, "%s%s", SystemValuesStr,TransmissionData);
}




