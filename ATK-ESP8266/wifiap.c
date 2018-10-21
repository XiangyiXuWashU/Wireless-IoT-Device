#include "Includes.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

//ATK-ESP8266 WIFI module WIFI AP driver
								  
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
extern u8 WIFISendData[1406];
extern u8 TransmissionData[1332];


//Initialize the WIFI module
void atk_8266_UDP_Init(void)
{
    u8 ipbuf[16]="192.168.4.2"; 	//IP Buffer
    u8 p[50];

    //Set WIFI REST+IO
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_SetBits(GPIOC, GPIO_Pin_9);	//REST Reset low active

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_SetBits(GPIOC, GPIO_Pin_8);  //IO High level operation mode low level programming mode

    SwitchDATALED(0);

    while(atk_8266_send_cmd("AT+CWMODE=2","OK",50));		//Set WIFI AP+STA mode
    atk_8266_send_cmd("AT+RST","OK",20);		//Restart module
    delay_ms(1000);         //Delay 2S waiting for the restart to succeed
    delay_ms(1000);
    delay_ms(1000);
    memset(p,0,50);
    sprintf((char *)p,"AT+CWSAP=\"%s\",\"%s\",1,4",wifiap_ssid,wifiap_password);//Set the wireless parameters of AP mode: ssid, password
    while(atk_8266_send_cmd(p,"OK",1000));					//Set AP mode parameters

    //UDP
    memset(p,0,50);
    sprintf((char *)p,"AT+CIPSTART=\"UDP\",\"%s\",%s",ipbuf,(u8*)portnum);    //Configure the target UDP server.
    while(atk_8266_send_cmd("AT+CIPMUX=0","OK",100));  //Single link mode
    while(atk_8266_send_cmd(p,"OK",500));

    USART3_RX_STA=0;
    memset(p,0,50);

    SwitchDATALED(1);  //WIFI configuration is successful
}


void SendDataToPhone(void)
{
    atk_8266_send_cmd("AT+CIPSEND=1406","OK",0); //Send data of the specified length
    delay_ms(1);                                 //Wait for WIFI transmission
    IWDG_ReloadCounter();                        //Watchdog reset
    GPIO_ResetBits(GPIOD, GPIO_Pin_15);          //DATA signal light
    atk_8266_send_data(WIFISendData,"OK",0);     //Send data of the specified length
    delay_ms(1);                                 //Wait for WIFI transmission
    GPIO_SetBits(GPIOD, GPIO_Pin_15);            //DATA signal light
    IWDG_ReloadCounter();                        //Watchdog reset
}
	


