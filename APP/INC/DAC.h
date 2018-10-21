#ifndef _DAC_H
#define _DAC_H

/* Private define ------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

void InitDAC(void);
void GPIO_DAC_Configuration(void);
void TIM2_Configuration(void);
void SetTemperature(float Temp);
void SetCurrent(double Bias, double Modulate);
void GenerateTriangle(double offset,double Amplitude);
	
#endif /*_DAC_H*/
