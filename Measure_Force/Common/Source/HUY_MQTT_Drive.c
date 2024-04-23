#include <stdio.h>
#include "mqtt_client.h"
#include "HUY_MQTT_Drive.h"

typedef void(*Function_Callback_t)(HUY_MQTT_Event_enum);
static Function_Callback_t Handler_Callback;

esp_mqtt_client_handle_t client_control;

char *MQTT_Topic_Receive;
char *MQTT_Data_Receive;
int MQTT_Topic_length=0;
int MQTT_Data_length=0;

static void MQTT_Event_Handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    HUY_MQTT_Event_enum Check_Event;
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    switch ((esp_mqtt_event_id_t)event_id) 
    {
        case MQTT_EVENT_CONNECTED: Check_Event = MQTT_Connected;
            break;
        case MQTT_EVENT_DISCONNECTED: Check_Event = MQTT_DisConnected;
            break;
        case MQTT_EVENT_SUBSCRIBED: Check_Event = MQTT_Subcribed;
            break;
        case MQTT_EVENT_UNSUBSCRIBED: Check_Event = MQTT_UnSubcribed;
            break;
        case MQTT_EVENT_PUBLISHED: Check_Event = MQTT_Published;
            break;
        case MQTT_EVENT_DATA: 
            MQTT_Topic_Receive = event->topic;
            MQTT_Topic_length = event->topic_len;
            MQTT_Data_Receive = event->data;
            MQTT_Data_length = event->data_len;
            Check_Event = MQTT_Received;
            break;
        case MQTT_EVENT_ERROR: Check_Event = MQTT_Error;
            break;
        default:
             Check_Event = MQTT_Connecting;
            break;
    }
    Handler_Callback(Check_Event);
}

void MQTT_Client_Start(char *Broker_Address,void (*Handler_Address))
{
    Handler_Callback = Handler_Address;
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = Broker_Address,};
    client_control = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client_control, ESP_EVENT_ANY_ID, MQTT_Event_Handler, NULL);
    esp_mqtt_client_start(client_control);
}

void MQTT_Reconnect()
{
    esp_mqtt_client_reconnect(client_control);
}

void MQTT_Disconnect()
{
    esp_mqtt_client_disconnect(client_control);
}

void MQTT_Client_Stop()
{
    esp_mqtt_client_stop(client_control);
}

void MQTT_Subcribe_Topic(char *Topic, uint8_t QOS)
{
    esp_mqtt_client_subscribe_single(client_control, Topic, QOS);
}

void MQTT_UnSubcribe_Topic(char *Topic)
{
    esp_mqtt_client_unsubscribe(client_control,Topic);
}
void MQTT_Pushlish_Data(char *Topic, char *Data, uint8_t QOS)
{
    esp_mqtt_client_publish(client_control,Topic,Data,0,QOS,0);
}


uint8_t String_Compare(const char *String1,const char *String2,int length)
{
    for(int i=0;i<length;i++)
    {
        if(*(String1+i)!=*(String2+i))
        {
            return 0;
        }
    }
    return 1;
}
