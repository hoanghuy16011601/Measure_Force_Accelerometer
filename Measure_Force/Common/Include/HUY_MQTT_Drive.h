#ifndef HUY_MQTT_Drive_h
#define HUY_MQTT_Drive_h
#include <stdint.h>


typedef enum{
    MQTT_Connecting = 0,
    MQTT_Connected = 1,
    MQTT_DisConnected = 2,
    MQTT_Subcribed =3,
    MQTT_UnSubcribed =4,
    MQTT_Published =5,
    MQTT_Received =6,
    MQTT_Error =7
}HUY_MQTT_Event_enum;


void MQTT_Client_Start(char *Broker_Address,void (*Handler_Address));
void MQTT_Reconnect();
void MQTT_Disconnect();
void MQTT_Client_Stop();
void MQTT_Subcribe_Topic(char *Topic, uint8_t QOS);
void MQTT_UnSubcribe_Topic(char *Topic);
void MQTT_Pushlish_Data(char *Topic, char *Data, uint8_t QOS);
uint8_t String_Compare(const char *String1,const char *String2,int length);
#endif