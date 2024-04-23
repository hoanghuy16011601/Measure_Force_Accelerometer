#include <stdio.h>
#include "Huy_PWM_Drive.h"
#include "HUY_GPIO_Drive.h"
#include "HUY_WIFI_Drive.h"
#include "HUY_HTTP_SERVER_Drive.h"
#include "mdns.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"


char* SSID_SAVED;
char* PASS_SAVED;


uint32_t Input_Degree =0;
float Error_Position =0;
float Error_Before =0;
float Total_Error=0;
uint32_t count=0;


TimerHandle_t TaskTimer[2];

typedef enum{
    Inverse_State =0,
    Forward_State,
    Stop_State,
}Direction_State_enum;
typedef enum{
    Start_State =0,
    Back_State,
    Free_State,
}Signal_State_enum;
Direction_State_enum Direction_State = Stop_State;
Signal_State_enum Module_State = Free_State;

Motor_PWM_enum Motor_PWM_Select;


/// 
///


void Inverse_Motor(float DutyCycle);
void Forward_Motor(float DutyCycle);
void Stop_Motor(void);
float PID_Function(void);
void Reset_PID(uint32_t Input);
void Run(uint32_t Input);
void Select_Motor(uint8_t Motor_Number);
/////

void TurnOn_Led(uint8_t Led);
void TurnOff_Ledd(void);
void Blynk(void);
void Find_SSID_PASS(char *String, char *SSID, char *PASS);


void vTimerCallback(TimerHandle_t xTimer)
{
    configASSERT (xTimer);
    int IdTimer = (int) pvTimerGetTimerID(xTimer);
    if (IdTimer == 0)
    {
        switch (Direction_State)
        {
            float DutyCycle;
            case Inverse_State: 
                DutyCycle = PID_Function();
                Inverse_Motor(DutyCycle);
                break;
            case Forward_State:
                DutyCycle = PID_Function();
                Forward_Motor(DutyCycle);
                break;
            default:
                Stop_Motor();
                break;
        }
    }
    

}

void GPIO_INTERRUPT_HANLDER(int PIN)
{
    if(PIN ==19)
    {
        count++;
    }
    else if (PIN ==21)
    {
        count++;
    }
    
}


void WiFi_Station_Handler(HUY_WIFI_STATION_EVENT_enum Event)
{
    static uint8_t TimeStationtrue =0;
    if (Event == WiFi_Station_FailConnected)
    {
        TurnOff_Ledd();
        WIFI_STATION_CONNECT(SSID_SAVED,PASS_SAVED);
    }
    
    else if (Event == WiFi_Station_GotIP)
    {
        mdns_init();
        mdns_hostname_set("myesp32");
        mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
        TurnOn_Led(12);
    }
    
}


void Config_WiFi_handler(httpd_req_t *request)
{
    char *Buffer = (char *)malloc(48*sizeof(char));
    char *SSID_CONNECTING = (char *)malloc(16*sizeof(char));
    char *PASS_CONNECTING = (char *)malloc(16*sizeof(char));
    memset(Buffer,0,48);
    httpd_req_recv(request, Buffer, 48);
    memset(SSID_CONNECTING,0,16);
    memset(PASS_CONNECTING,0,16);
    Find_SSID_PASS(Buffer,SSID_CONNECTING,PASS_CONNECTING);
    WIFI_STATION_CONNECT(SSID_CONNECTING,PASS_CONNECTING);
    TurnOff_Ledd();
    free(Buffer);
    free(SSID_CONNECTING);
    free(PASS_CONNECTING);
}

void start_handler(httpd_req_t *request){
    Module_State = Start_State;
}

void back_handler(httpd_req_t *request)
{
    Module_State = Back_State;
}

void checkconnect_handler( httpd_req_t *request)
{
    Blynk();
    TurnOn_Led(13);
}


void app_main()
{
    Configure_Output_Mode(13);
    Configure_Output_Mode(12);
    Configure_Output_Mode(26);
    Configure_Output_Mode(27);
    SSID_SAVED = (char *)"MEASURE FORCE";
    PASS_SAVED = (char *)"0123456789";
    WiFi_Init();
    WIFI_STATION_INIT(WiFi_Station_Handler);
    WIFI_STATION_CONNECT(SSID_SAVED,PASS_SAVED);
    Server_Run();
    Server_Initalization("/start",start_handler,HTTP_POST);
    Server_Initalization("/back",back_handler,HTTP_POST);
    Server_Initalization("/configwifi",Config_WiFi_handler,HTTP_POST);
    Server_Initalization("/checkconnect",checkconnect_handler,HTTP_POST);
    PWM_init(5000,Timer1,Channel0,18);
    PWM_init(5000,Timer2,Channel1,5);
    Select_Motor(0);
    Configure_Interrupt_GPIO(19,Rising_Edge,Pullup_Disable,Pulldown_Enable);
    Configure_Interrupt_GPIO(21,Rising_Edge,Pullup_Disable,Pulldown_Enable);
    Function_GetAdress(GPIO_INTERRUPT_HANLDER);
    TaskTimer[0] = xTimerCreate("TimerTask0",pdMS_TO_TICKS(10),pdTRUE,(void *)0,vTimerCallback);
    while (1)
    {
        switch (Module_State)
        {
        case Start_State:
            Select_Motor(1);
            Direction_State = Inverse_State;
            Run(90);
            break;
        case Back_State:
            Select_Motor(1);
            Direction_State = Forward_State;
            Run(90);
        default:
            break;
        }
    }
    
}

//////

void Select_Motor(uint8_t Motor_Number)
{

    //// 26 Motor1 (Gate)
    //// 27 Motor2 (rope)
    if (Motor_Number ==1)
    {
        Motor_PWM_Select = Motor_PWM1;
        Write_GPIO(27,LOW);
        Write_GPIO(26,HIGH);
    }
    else if (Motor_Number ==2)
    {
        Motor_PWM_Select = Motor_PWM2;
        Write_GPIO(27,HIGH);
        Write_GPIO(26,LOW);
    }
    else
    {
        Motor_PWM_Select = No_Motor;
        Write_GPIO(27,LOW);
        Write_GPIO(26,LOW);
    }
    
    
}

void Inverse_Motor(float DutyCycle)
{
    PWM_Update_DutyCycle(Channel1,0,Motor_PWM_Select);
    PWM_Update_DutyCycle(Channel0,DutyCycle,Motor_PWM_Select);
}

void Forward_Motor(float DutyCycle)
{
    PWM_Update_DutyCycle(Channel1,DutyCycle,Motor_PWM_Select);
    PWM_Update_DutyCycle(Channel0,0,Motor_PWM_Select);
}

void Stop_Motor()
{
    PWM_Update_DutyCycle(Channel1,0,Motor_PWM_Select);
    PWM_Update_DutyCycle(Channel0,0,Motor_PWM_Select);
}

float PID_Function(void)
{
    Error_Position = (float)(Input_Degree - (count/1050.0)*360);
    Total_Error += Error_Position*0.1;
    float Duty_Cycle = (Error_Position*0.5 + (Error_Position - Error_Before)*1 + Total_Error*0.1)/90;
	Error_Before = Error_Position;
    return Duty_Cycle;
}

void Reset_PID(uint32_t Input)
{
    Input_Degree = Input;
    count =0;
    Error_Position =Input;
    Total_Error =0;
    Error_Before =0;
}

void Run(uint32_t Input)
{
    Reset_PID(Input);
    xTimerStart(TaskTimer[0],0);
    while (Error_Position>0);
    Stop_Motor();
    Direction_State = Stop_State;
    vTaskDelay(20/portTICK_PERIOD_MS);
    xTimerStop(TaskTimer[0],0);
    Select_Motor(0);
    Module_State = Free_State;
}


/////

void TurnOn_Led(uint8_t Led)
{
    ///// Led13 Connect
    ///// Led12 WIFI
    Write_GPIO(Led,HIGH);
}

void TurnOff_Ledd(void)
{
    Write_GPIO(13,LOW);
    Write_GPIO(12,LOW);
}

void Blynk(void)
{
    Write_GPIO(13,HIGH);
    vTaskDelay(200/portTICK_PERIOD_MS);
    Write_GPIO(13,LOW);
    vTaskDelay(200/portTICK_PERIOD_MS);
    Write_GPIO(13,HIGH);
    vTaskDelay(200/portTICK_PERIOD_MS);
    Write_GPIO(13,LOW);
    vTaskDelay(200/portTICK_PERIOD_MS);
    Write_GPIO(13,HIGH);
    vTaskDelay(200/portTICK_PERIOD_MS);
    Write_GPIO(13,LOW);
    vTaskDelay(200/portTICK_PERIOD_MS);

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
