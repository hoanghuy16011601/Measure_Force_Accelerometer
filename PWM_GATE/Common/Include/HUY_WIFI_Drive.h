#ifndef HUY_WIFI_DRIVE_H
#define HUY_WIFI_DRIVE_h
#include <stdbool.h>
#include <stdint.h>

typedef enum{
    Smart_Config_Default =-1,
    Smart_Config_Found_Channel =0,
    Smart_Config_Got_SSID_PASS =1,
    Smart_Config_Connected_WIFI =2,
    Smart_Config_Over =3,
}HUY_SMART_CONFIG_EVENT_enum;

typedef enum{
    WiFi_Station_Default =0,
    WiFi_Station_Connected =1,
    WiFi_Station_FailConnected =2,
    WiFi_Station_GotIP =3,
}HUY_WIFI_STATION_EVENT_enum;



void WiFi_Init(void);
void WIFI_STATION_INIT(void(*WiFi_Station_Event_Handler_Address));
void WIFI_STATION_STOP(void);
void WIFI_STATION_CONNECT(char *SSID,char *PASSWORD);
void smart_config_WiFi_station(void (*Handler_Address));
void WIFI_AP_INIT(void);
void WIFI_AP_STOP(void);
void STATIC_IP(void);
#endif