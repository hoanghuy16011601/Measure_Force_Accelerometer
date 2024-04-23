#ifndef HUY_PWM_DRIVE_H
#define HUY_PWM_DRIVE_H

#include<stdint.h>

typedef enum{
    Timer1 = 0,
    Timer2,
    Timer3,
    Timer_Max,
}TIMER_CHANNEL_enum;

typedef enum{
    Channel0 =0,
    Channel1,
    Channel2,
    Channel3,
}PWM_CHANNEL_enum;

typedef enum{
    Motor_PWM1 =0,
    Motor_PWM2,
    No_Motor,
}Motor_PWM_enum;


void PWM_init(uint32_t Frequency,TIMER_CHANNEL_enum Timer_Channel,PWM_CHANNEL_enum Channel_Config, uint8_t GPIO_Number);
void PWM_Update_DutyCycle(PWM_CHANNEL_enum Channel_Config,float Percent,Motor_PWM_enum Motor_PWM);

#endif