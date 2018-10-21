#ifndef __COMMON_H__
#define __COMMON_H__	 

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//ATK-ESP8266 WIFI Driver
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

u8 atk_8266_mode_cofig(u8 netpro);
void atk_8266_at_response(u8 mode);
u8* atk_8266_check_cmd(u8 *str);
u8 atk_8266_apsta_check(void);
u8 atk_8266_send_data(u8 *data,u8 *ack,u16 waittime);
u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime);
u8 atk_8266_quit_trans(void);
u8 atk_8266_consta_check(void);

void atk_8266_get_wanip(u8* ipbuf);

//User configuration parameters
extern const u8* portnum;		//Connection port
 
extern const u8* wifista_ssid;		//WIFI STA SSID
extern const u8* wifista_encryption;    //WIFI STA Encryption
extern const u8* wifista_password; 	//WIFI STA password

extern const u8* wifiap_ssid;		//WIFI AP SSID
extern const u8* wifiap_encryption;	//WIFI AP Encryption
extern const u8* wifiap_password; 	//WIFI AP password

extern u8* kbd_fn_tbl[2];
extern const u8* ATK_ESP8266_CWMODE_TBL[3];
extern const u8* ATK_ESP8266_WORKMODE_TBL[3];
extern const u8* ATK_ESP8266_ECN_TBL[5];
#endif





