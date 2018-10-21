#include "Includes.h"

extern u8 WIFISendData[1406];     //All data sending to iPhone
extern u8 TransmissionData[1332]; //Spectrum data sending to iphone
															
int main (void){
    InitMainBoard();

    while(1){
        SendDataToPhone();  //Send data to iphone
    }
}

#define USE_FULL_ASSERT

#ifdef  USE_FULL_ASSERT
 
void assert_failed(uint8_t* file, uint32_t line){
    while (1){

    }
}

#endif


