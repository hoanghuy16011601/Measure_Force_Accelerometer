
#include "lwip/netif.h"
#include "mdns.h"
#include "HUY_WIFI_Drive.h"
#include "HUY_HTTP_SERVER_Drive.h"
#include "HUY_MQTT_Drive.h"
#include "HUY_I2C_Drive.h"
#include "HUY_MPU6050_Drive.h"
#include "HUY_HX711_Drive.h"
#include "HUY_GPIO_Drive.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

typedef enum{
    Start_Measure_Accel =0,
    Stop_Measure_Accel,
}Flag_Measure_Accel;
Flag_Measure_Accel Acceleremetor_Flag = Stop_Measure_Accel;

uint8_t Time_Station_True =0;
float Acceleremetor =0;
float Zero =0;
float Force =0;

void Buzzer(uint32_t MS)
{
    Write_GPIO(25,HIGH);
    vTaskDelay(MS/portTICK_PERIOD_MS);
    Write_GPIO(25,LOW);
}

void Find_SSID_PASS(char *String, char *SSID, char *PASS)
{
    int i =5;
    char *start_tag, *finish_tag;
    start_tag = strstr(String, "ssid:");
    finish_tag = strstr(String, "pass:");

    while(*(start_tag+i)!= *(finish_tag))
    {
        *(SSID+i-5)=*(start_tag+i);
        i++;
    }
    start_tag = strstr(String, "pass:");
    finish_tag = strstr(String, "end@");
    i =5;
    while(*(start_tag+i)!=*finish_tag)
    {
        *(PASS+i-5)=*(start_tag+i);
        i++;
    }
}



void Webclient_handler(httpd_req_t *request)
{
    httpd_resp_set_type(request,"text/html");
    httpd_resp_send(request,(const char*) index_html_start, index_html_end-index_html_start);
}

void WIFI_STATION_HANDLER(HUY_WIFI_STATION_EVENT_enum Event)
{
    if (Event == WiFi_Station_GotIP)
    {
        Time_Station_True =1;
    }
    else if (Event == WiFi_Station_FailConnected)
    {
        Time_Station_True =0;
        WIFI_STATION_STOP();
        WIFI_AP_INIT();
        Buzzer(3000);
    }
    
}

void getvalue_handler(httpd_req_t *request){
    char Respone_string[64];
    sprintf(Respone_string,"{\"a\":\"%f\",\"f\":\"%f\"}",Acceleremetor,Force);
    httpd_resp_send(request,Respone_string,strlen(Respone_string));
}


void start_measure_handler(httpd_req_t *request){
    char Respone_string[] = "OKAY";
    uint8_t Data_Buffer[3] = {0,0,NULL};
    Force =0;
    float Data_Measure =(float) hx711_measure()/10000.0;
    Force = (Data_Measure - Zero)*0.25;
    httpd_resp_send(request,Respone_string,strlen(Respone_string));
    Acceleremetor_Flag = Start_Measure_Accel;
    Acceleremetor =0;
}

void ConfigWifi_handler(httpd_req_t *request)
{
    char *Buffer = (char *)malloc(64*sizeof(char));
    char *SSID_CONNECTING = (char *)malloc(64*sizeof(char));
    char *PASS_CONNECTING = (char *)malloc(64*sizeof(char));
    memset(Buffer,0,48);
    memset(SSID_CONNECTING,0,16);
    memset(PASS_CONNECTING,0,16);
    httpd_req_recv(request, Buffer, 48);
    Find_SSID_PASS(Buffer,SSID_CONNECTING,PASS_CONNECTING);
    if (Time_Station_True ==0)
    {
        WIFI_AP_STOP();
        WIFI_STATION_INIT(WIFI_STATION_HANDLER);
    }
    WIFI_STATION_CONNECT(SSID_CONNECTING,PASS_CONNECTING);
    free(Buffer);
    free(SSID_CONNECTING);
    free(PASS_CONNECTING);
}

void app_main(void)
{
    Configure_Output_Mode(14);
    Configure_Output_Mode(25);
    Write_GPIO(14,HIGH);
    hx711_init(12,13);
    WiFi_Init();
    WIFI_AP_INIT();
    Server_Run();
    mdns_init();
    mdns_hostname_set("vuthanhbinh");
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
    Server_Initalization("/web",Webclient_handler,HTTP_GET);
    Server_Initalization("/getvalue",getvalue_handler,HTTP_GET);
    Server_Initalization("/start_measure",start_measure_handler,HTTP_GET);
    Server_Initalization("/configwifi",ConfigWifi_handler,HTTP_POST);
    I2C_Master_Initialization(0,22,21,100000);
    MPU6050_Initialization(0,0x68);
    Zero = (float) hx711_measure();
    Zero = (float) hx711_measure();
    Zero = (float) hx711_measure()/10000.0;

    
    while (1)
    {
        if (Acceleremetor_Flag == Start_Measure_Accel)
        {
            float Ax_Before =0;
            uint8_t count =0;
            uint8_t count_Same = 0;
            uint16_t Count_10ms =0;
            int16_t Ax_RAW =0;
            float Ax =0.0;
            uint8_t Data_Buffer[3] = {0,0,NULL};
            Acceleremetor =0;
            while (Acceleremetor_Flag == Start_Measure_Accel)
            {
                MPU6050_ReadValue(0,0x68,59,&Data_Buffer[0]);
                MPU6050_ReadValue(0,0x68,60,&Data_Buffer[1]);
                Ax_RAW = (int16_t) Data_Buffer[0]<<8|Data_Buffer[1];
                Ax = Ax_RAW/4096.0;
                if (Ax>0.07 && abs(Ax - Ax_Before <0.02))
                {
                    count_Same ++;
                    if (count_Same ==5)
                    {
                        Ax =0;
                        count =0;
                        while (count <2)
                        {
                            vTaskDelay(100/portTICK_PERIOD_MS);
                            MPU6050_ReadValue(0,0x68,59,&Data_Buffer[0]);
                            MPU6050_ReadValue(0,0x68,60,&Data_Buffer[1]);
                            Ax_RAW = (int16_t) Data_Buffer[0]<<8|Data_Buffer[1];
                            Ax += Ax_RAW/4096.0;
                            count++; 
                        }
                        Acceleremetor = Ax/0.2;
                        Acceleremetor_Flag = Stop_Measure_Accel;
                        break;
                    }
                    
                }
                else
                {
                    count_Same =0;
                }
                Ax_Before = Ax;
                vTaskDelay(20/portTICK_PERIOD_MS);
                Count_10ms ++;
                if (Count_10ms >500)
                {
                    Acceleremetor_Flag = Stop_Measure_Accel;
                    break;
                }
            }
        }
            
        
    }
    
    /*
    while (1)
    {
        float Ax_Before =0;
        uint8_t count =0;
        uint8_t count_Same = 0;
        uint16_t Count_10ms =0;
        int16_t Ax_RAW =0;
        float Ax =0.0;
        uint8_t Data_Buffer[3] = {0,0,NULL};
        Acceleremetor =0;
        printf("Start\n");
        while (1)
        {
        
            MPU6050_ReadValue(0,0x68,59,&Data_Buffer[0]);
            MPU6050_ReadValue(0,0x68,60,&Data_Buffer[1]);
            Ax_RAW = (int16_t) Data_Buffer[0]<<8|Data_Buffer[1];
            Ax = Ax_RAW/4096.0;
            if (Ax>0.1 && abs(Ax - Ax_Before <0.002))
            {
                count_Same ++;
                if (count_Same ==)
                {
                    Ax =0;
                    count =0;
                    while (count <2)
                    {
                        vTaskDelay(100/portTICK_PERIOD_MS);
                        MPU6050_ReadValue(0,0x68,59,&Data_Buffer[0]);
                        MPU6050_ReadValue(0,0x68,60,&Data_Buffer[1]);
                        Ax_RAW = (int16_t) Data_Buffer[0]<<8|Data_Buffer[1];
                        Ax += Ax_RAW/4096.0;
                        count++; 
                    }
                    Acceleremetor = Ax/0.2;
                    printf("%f\n",Acceleremetor);
                    Acceleremetor_Flag = Stop_Measure_Accel;
                    vTaskDelay(5000/portTICK_PERIOD_MS);
                    printf("Start\n");
                }
                
            }
            else
            {
                count_Same =0;
            }
            Ax_Before = Ax;
            vTaskDelay(20/portTICK_PERIOD_MS);
            
            Count_10ms ++;
            if (Count_10ms >5000)
            {
                Acceleremetor_Flag = Stop_Measure_Accel;
                break;
            }
            
        }
        
    }
    */
    
}
