#ifndef _ADC_H
#define _ADC_H

/* Private define ------------------------------------------------------------*/
#define ADC1_DR_Address    ((u32)0x4001244C)

/* Private function prototypes -----------------------------------------------*/
void ADC_Configuration(void);
void Synchronize_ADC_DAC(void);
void ReadVoltage(void);
void Calculate_LaserProtect_CombineSendValue(void);

#endif /*_ADC_H*/
