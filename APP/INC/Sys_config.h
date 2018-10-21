#ifndef  _SYS_CONFIG_H
#define  _SYS_CONFIG_H

void InitMainBoard(void);
void NVIC_Configuration(void);
void SwitchDATALED(int State);
void SwitchTEC(int State);
void SwitchLaser(int State);
double LimitRangeValue(double input, double min, double max, double defaultValue, int isUseDefaultOutrange);
void InitEXTRALED(void);
void FlashEXTRALED(void);

#endif
