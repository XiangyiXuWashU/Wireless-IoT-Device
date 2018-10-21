#include "Includes.h"

SEQUEUE RX; //Receive queue
SEQUEUE TX; //Send queue

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

Recv ComFrame;//eceiving structure, storing received data
unsigned char Comcode;//save command code pointer

u8  Flag_head = 0;   //Judge the USART1 frame header flag
u8  Flag_rx_len = 0; //Receive frame length flag
u16 Slave_length =0; //data length
u8  Slave_check = 0; //checksum
u8  RxBuffer[64];   //Receive data
u16 RxCounter;      //Receive counter

extern  const unsigned int Resistance[23];

extern volatile unsigned  short int  PD_ADCValue[1000];
extern volatile unsigned  short int  ADC_ResultValue[6];

extern u8 FlashData[8];    //used to store data to be stored in Flash

extern u8 WIFISendData[1406]; //All data sent to the phone
extern u8 TransmissionData[1332]; //Spectrum data sent to the phone

extern double Temperature;
extern double BiasCurrent;	
extern double ModulateCurrent;

extern u8 startReadWifiCommondData;  //read the data sent from the phone
extern u8 startReadWifiCommondDataFlag;
extern u8 stopReadWifiCommondDataFlag;

extern u8 startProcessWifiCommondData;      //start executing the command sent by the phone
extern u8 startProcessWifiCommondDataFlag;  //used to determine the end of the command sent by the phone
extern u8 IPHONE_COMMOND_BUF[30];           //used to store pending commands from the phone
extern u8 IPHONE_COMMOND_BUF_INDEX;

#define  ALPHABET_X    0x58
#define  ALPHABET_Y    0x59
#define  ALPHABET_Z    0x5A
#define  ALPHABET_C    0x43

void InitData(void)
{
  RX.front = RX.rear = 0;
  TX.front = TX.rear = 0;
}	

/********************************************************************
Function function: queue enqueue, receive data
Entry parameters:
Return:
Remarks:
********************************************************************/
u8 rx_enqueue(u8 x)             
{
     if(RX.front == (RX.rear+1)%BUFF_MAX)
        return 0;
     else
       {
        RX.uart_buff[RX.rear] = x;
        RX.rear = (RX.rear+1)%BUFF_MAX;
        return 0;
       }
}

/********************************************************************
Function: queue dequeue, receive data
Entry parameters:
Return:
Remarks: When dequeuing, first judge the length of L, whether it is greater than 0. Otherwise, you can't leave the team.
********************************************************************/
u8 rx_dequeue(void)   
{
     u8 temp;

     if(RX.front == RX.rear)
        return 0;
     else
     {
        temp = RX.uart_buff[RX.front];
        RX.front = (RX.front+1)%BUFF_MAX;

        return temp;
     }
}
/********************************************************************
Function function: queue enqueue, send buffer
Entry parameters:
Return:
Remarks:
********************************************************************/
u8 tx_enqueue(u8 x)
{
    if(TX.front == (TX.rear+1)%BUFF_MAX)//queue full
    return 0;
    else
    {
        TX.uart_buff[TX.rear] = x;
        TX.rear = (TX.rear+1)%BUFF_MAX;
        return 1;
    }
}
/********************************************************************
Function: queue dequeue, send buffer
Entry parameters:
Return:
Remarks:
********************************************************************/
u8 tx_dequeue(void)               //queue pop
{
     u8 temp;
     if(TX.front == TX.rear) //eqeue emty
        return 0;
     else
     {
      temp = TX.uart_buff[TX.front];
      TX.front = (TX.front+1)%BUFF_MAX;

      return temp;  //Return the first element of the queue
     }
}

//Send by USB
void USART_Send_Begin(u8* buff,u16 count)
{
    u32 remain = count;
    u32 index = 0;

    while (remain > 63) //Send up to 64 bytes at a time
    {
        while(GetEPTxStatus(ENDP1)!=EP_TX_NAK);

        UserToPMABufferCopy(&buff[index], ENDP1_TXADDR, 64);

        SetEPTxCount(ENDP1, 64);

        SetEPTxValid(ENDP1);

        index += 64; //Calculate the offset and the number of remaining bytes

        remain -= 64;
    }

    while(GetEPTxStatus(ENDP1)!=EP_TX_NAK);

    //Send the last <=64 bytes of data

    UserToPMABufferCopy(&buff[index], ENDP1_TXADDR, remain);

    SetEPTxCount(ENDP1, remain);

    SetEPTxValid(ENDP1);
}

//----------------------------------------------------------
// Function: This function will store the received data into the receiving structure
// Input: Rbuffer - store the received data
// Output: ComFrame - the structure of the receiving data store
//-----------------------------------------------------------
void RecvDepackage(Recv *ComFrame,unsigned char Rbuffer[])
{
   int i,j;
   ComFrame->Rhd[0]=Rbuffer[0];
   ComFrame->Rhd[1]=Rbuffer[1];

   j=ComFrame->Rhd[1]-4;	         //Receive data byte length

   for(i=0;i<j;i++)			 //Receive data
   {
      ComFrame->Rda[i]=Rbuffer[i+2];  
   }
   ComFrame->Rend[0]=Rbuffer[j+2];	 //Receive check code
   ComFrame->Rend[1]=Rbuffer[j+3];       //Receive end code
}

/********************************************************************
Function: Unpacking the data in the queue
Entry parameters:
Return:
Remarks: Rxbuffer
********************************************************************/
void Process_cmd(void)
{
   int i=0;
   u8 Data;
   u8 Check_ok;

   if(RX.front != RX.rear)//Indicates that the serial queue is not empty.
   {
        Data = rx_dequeue();//Take a data from the queue

        if(!Flag_head)
          {
           if((Data == SLAVE_HEAD1)||(Data == SLAVE_HEAD2)||(Data == SLAVE_HEAD3)) //Receiving frame header
            {
                RxCounter = 0;
                for(i=0;i<UART_BUFFERSIZE;i++){RxBuffer[i]='\0'; } //clear RxBuffer
                RxBuffer[RxCounter++] = Data;      //Receive frame header data
                Flag_head = 1;
                Flag_rx_len = 0;
                Slave_check=Data;  //Calculate checksum
            }
          }
       else if(!Flag_rx_len) //Receive data length
        {
            RxBuffer[RxCounter++] = Data;      //Receive length data
            Flag_rx_len = 1;
            Slave_length = Data;
            Slave_check^=Data;    //Calculate checksum
        }
       else
        {
            RxBuffer[RxCounter++] = Data;      //Receive data

            if(RxCounter <= (Slave_length-2))
                {
                    Slave_check^=Data;        //Calculate checksum
                }

            if(RxCounter == (Slave_length-1)) //Data reception and comparison
               {
                    Slave_check=~Slave_check;//Calculate checksum
               }

            if(RxCounter == Slave_length) //Data reception completed
                 {
                   RxCounter=0;

                   if((RxBuffer[Slave_length -2] == Slave_check)&&(RxBuffer[Slave_length -1]==0x00))
                     {
                      Check_ok = 1; //Checksum is correct
                      Data_Poll(Check_ok);
                     }

                   Flag_head = 0;
                   Flag_rx_len = 1;
                 }
            if(RxCounter > Slave_length) //Receiving error conditions
                 {
                    RxCounter=0;
                    Flag_head = 0;
                    Flag_rx_len = 1;
                 }
            }
         }
}

//////////////////////////////////////////
//////////////////////////////////////////

unsigned char addr;	
unsigned char Data[300];
u8 Buff_temp[2000];

void Data_Poll(unsigned char Check_ok)
{  	
  unsigned char Datalength;
  unsigned char slave_addr;	
  int i=0;
  
  if(Check_ok)
  {
        RecvDepackage(&ComFrame,RxBuffer);//The data in the receive buffer is stored in the receive structure

        Comcode=ComFrame.Rhd[0];
        Datalength=ComFrame.Rhd[1]-6;
        slave_addr = ComFrame.Rda[0];
        addr = ComFrame.Rda[1];
        for(i=0;i<Datalength;i++)
        {
            Data[i] = ComFrame.Rda[i+2];
        }

        if(Comcode==0x20)	 //Control register write data command
        {

        }

        else if(Comcode==0x21)	 //Control register to read data commands
        {

        }

        else if(Comcode==0x22)	 //Read Transmission Spectrum
        {
            switch(slave_addr)
             {
                case(0x00):   //Read Spectrum

                         for (i = 0; i < 1000; i++)
                            {
                                Buff_temp[i*2]=(PD_ADCValue[i]>>8)&0xFF;
                                Buff_temp[i*2+1]=PD_ADCValue[i]&0xFF;

                             }

                         USART_Send_Begin(Buff_temp,2000);  //USB Sent

                         break;

                default:
                         break;
             }

        }
    }
}

//Handling commands sent by mobile phone Command format: XY+ command itself +ZC
void WIFIReceivedCommandProcess(void)
{
    int action = 0;
    u8 tempData;

    if(TX.front != TX.rear)
       {
         if(abs(TX.rear-TX.front)>500)
         {
           TX.front = TX.rear = 0;
           startReadWifiCommondData =0;
           startReadWifiCommondDataFlag =0;
           stopReadWifiCommondDataFlag =0;

           startProcessWifiCommondDataFlag=0;
           startProcessWifiCommondData=0;
         }
        tempData = tx_dequeue();
        IPHONE_COMMOND_BUF[IPHONE_COMMOND_BUF_INDEX++] = tempData;

        if(IPHONE_COMMOND_BUF_INDEX>=255)
        {
          IPHONE_COMMOND_BUF_INDEX=0;
        }

        if(tempData==0x5A) startProcessWifiCommondDataFlag=1;      //Determine if it is the letter Z
        else if(tempData==0x43&&startProcessWifiCommondDataFlag==1)//Determine if the second character is C
        {
           startProcessWifiCommondData=1;
           IPHONE_COMMOND_BUF_INDEX=0;
        }
        else
        {
           startProcessWifiCommondDataFlag=0;
           startProcessWifiCommondData=0;
        }
    }

    if(startProcessWifiCommondData)
    {
        if      (strstr((const char*)IPHONE_COMMOND_BUF,"LDON"))     {action = 1;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"LDOFF"))    {action = 2;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"TECON"))    {action = 3;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"TECOFF"))   {action = 4;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"LD+I"))     {action = 5;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"LD-I"))     {action = 6;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"LD+T"))     {action = 7;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"LD-T"))     {action = 8;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"S+I"))      {action = 9;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"S-I"))      {action = 10;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"LI:"))      {action = 11;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"SI:"))      {action = 12;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"LT:"))      {action = 13;}
        else if (strstr((const char*)IPHONE_COMMOND_BUF,"FLASH"))    {action = 14;}
        else
            {
             action =0;
            }

        switch(action)
         {
             case(1):   //Turn on LD
                SwitchLaser(1);
                FlashEXTRALED();
                break;

             case(2):   //Turn off LD
                SwitchLaser(0);
                FlashEXTRALED();
                break;

             case(3):   //Turn on TEC
                SwitchTEC(1);
                FlashEXTRALED();
                break;

             case(4):   //Turn off TEC
                 SwitchTEC(0);
                 FlashEXTRALED();
                 break;

             case(5):   //Increase LD Current
                BiasCurrent=BiasCurrent+1;
                BiasCurrent = LimitRangeValue(BiasCurrent, 0, 180, 120,0);
                ModulateCurrent = LimitRangeValue(ModulateCurrent, 0, 40, 40,0);
                SetCurrent(BiasCurrent, ModulateCurrent);
                FlashEXTRALED();
                break;

             case(6):   //Decrease LD Current
                BiasCurrent=BiasCurrent-1;
                BiasCurrent = LimitRangeValue(BiasCurrent, 0, 180, 120,0);
                ModulateCurrent = LimitRangeValue(ModulateCurrent, 0, 40, 40,0);
                SetCurrent(BiasCurrent, ModulateCurrent);
                FlashEXTRALED();
                break;

             case(7):   //Increase LD Temperature
                Temperature=Temperature+0.1;
                Temperature = LimitRangeValue(Temperature, 10, 50, 25,0);
                SetTemperature(Temperature);
                FlashEXTRALED();
                break;

             case(8):   //Decrease LD Temperature
                Temperature=Temperature-0.1;
                Temperature = LimitRangeValue(Temperature, 10, 50, 25,0);
                SetTemperature(Temperature);
                FlashEXTRALED();
                break;

             case(9):   //Increase Scan Current
                  ModulateCurrent=ModulateCurrent+2;
                  ModulateCurrent = LimitRangeValue(ModulateCurrent, 0, 40, 40,0);
                  BiasCurrent = LimitRangeValue(BiasCurrent, 0, 180, 120,0);
                  SetCurrent(BiasCurrent, ModulateCurrent);
                  FlashEXTRALED();
                  break;

             case(10):   //Decrease Scan Current
                ModulateCurrent=ModulateCurrent-2;
                ModulateCurrent = LimitRangeValue(ModulateCurrent, 0, 40, 40,0);
                BiasCurrent = LimitRangeValue(BiasCurrent, 0, 180, 120,0);
                SetCurrent(BiasCurrent, ModulateCurrent);
                FlashEXTRALED();
                break;

             case(11):   //Set LD Current
                BiasCurrent = strtod (strstr((const char*)IPHONE_COMMOND_BUF,"LI:")+3,NULL);
                BiasCurrent = LimitRangeValue(BiasCurrent, 0, 180, 120,0);
                ModulateCurrent = LimitRangeValue(ModulateCurrent, 0, 40, 40,0);
                SetCurrent(BiasCurrent, ModulateCurrent);
                FlashEXTRALED();
                break;

             case(12):   //Set SCAN Current
                ModulateCurrent = strtod (strstr((const char*)IPHONE_COMMOND_BUF,"SI:")+3,NULL);
                ModulateCurrent = LimitRangeValue(ModulateCurrent, 0, 40, 40,0);
                BiasCurrent = LimitRangeValue(BiasCurrent, 0, 180, 120,0);
                SetCurrent(BiasCurrent, ModulateCurrent);
                FlashEXTRALED();
                break;

             case(13):   //Set LD Temperature
                Temperature = strtod (strstr((const char*)IPHONE_COMMOND_BUF,"LT:")+3,NULL);
                Temperature = LimitRangeValue(Temperature, 10, 50, 25,0);
                SetTemperature(Temperature);
                FlashEXTRALED();
                break;

             case(14):   //Save Parameters to Flash
                FlashProcess();
                FlashEXTRALED();
                break;

             default:

               break;
         }
        startProcessWifiCommondData=0;
        memset(IPHONE_COMMOND_BUF,0,256);
   }

}

//Intercept the Iphone command at the end of the XY opening ZC
void judgeIsIphoneSendCommond(u8 inputData)
{
    if(inputData == ALPHABET_X)
    {
     startReadWifiCommondDataFlag=1; //Determine if it is the letter X
    }
    else if((inputData == ALPHABET_Y)&&(startReadWifiCommondDataFlag == 1))//Determine if the second character is Y
    {
     startReadWifiCommondData = 1;
     tx_enqueue(ALPHABET_X);
    }
    else
    {
      startReadWifiCommondDataFlag=0;
    }

    if(inputData == ALPHABET_Z)
    {
       stopReadWifiCommondDataFlag = 1;//Determine if it is the letter Z
    }
    else if((inputData == ALPHABET_C)&&(stopReadWifiCommondDataFlag == 1))//Determine if the second character is C
    {
       startReadWifiCommondData = 0;
       tx_enqueue(inputData);
    }
    else
    {
       stopReadWifiCommondDataFlag=0;
    }

    if(startReadWifiCommondData)
    {
       tx_enqueue(inputData);
    }

}

double CalVS(int ADCValue)
{
    double Volt;
    Volt = (double)ADCValue*0.003;
    return Volt;
}

double CalVMCU(int ADCValue)
{
    double Volt;
    Volt = (double)ADCValue*0.002;
    return Volt;
}

double CalCurrent(int ADCValue)
{
    double Current;
    Current = (double)(ADCValue-1250)/10.045;
    return Current;
}

double CalTemp(int ADCValue)
{
    double Temp,R;
    int i=0;
    float k=0,b=0;

    R = (double)ADCValue*10;

    if(R<=682)
       Temp = 100 ;
    else if(R>=55786)
       Temp = -10;
    else
      {
        while(!((R <=  Resistance[i])&&(R> Resistance[i+1])))
        {
          i++;
        }

        if(R == Resistance[i])  Temp = i*5-10;

        else
        {
          k = (float)(Resistance[i]-Resistance[i+1])/5;
          b = Resistance[i]*(i-1)-(Resistance[i+1]*(i-2)) ;
          Temp = (b - R)/k;
        }
      }

    return Temp;
}

double CalVLD(int ADCValue)
{
    double Volt;
    Volt = (double)ADCValue*0.001;
    return Volt;
}

double mWtoDBM(double PmW)  
{
    double PdBm;
    PdBm = 10*log10(PmW);
    return PdBm;
}


//Inputdata: the shaping array to be converted
//number: the number to convert
//str: the target string to be converted
u8 *ConvertIntArrayToString(int *inputdata,int number,u8 *str)
{
    int i;
    u8 c1, c2, c3, c4;
    u8 temp;

    for (i = 0; i < number; i++)
    {
        temp=(inputdata[i]>>8)&0xFF;
        c1 = temp & 0xFu;
        c2 = (temp >> 4) & 0xFu;

        temp=inputdata[i]&0xFF;
        c3 = temp & 0xFu;
        c4 = (temp >> 4) & 0xFu;
        sprintf((char *)str + i * 4, "%X%X%X%X", c2, c1, c4, c3);
    }

    return str;
}




