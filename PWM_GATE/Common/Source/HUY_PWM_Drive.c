
#include <stdio.h>
#include <math.h>
#include "driver/ledc.h"
#include "Huy_PWM_Drive.h"
#include "esp_err.h"

#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits


void PWM_init(uint32_t Frequency,TIMER_CHANNEL_enum Timer_Channel,PWM_CHANNEL_enum Channel_Config, uint8_t GPIO_Number)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = Timer_Channel,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = Frequency,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = Channel_Config,
        .timer_sel      = Timer_Channel,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = GPIO_Number,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void PWM_Update_DutyCycle(PWM_CHANNEL_enum Channel_Config,float Percent,Motor_PWM_enum Motor_PWM)
{
    if (Motor_PWM == Motor_PWM1)
    {
        if (Percent <= 0)
        {
            Percent =0;
        }
        else if (Percent >=0.5)
        {
            Percent =0.5;
        } 
    }
    
    else if (Motor_PWM == Motor_PWM2)
    {
        if (Percent <= 0)
        {
            Percent =0;
        }
        else if (Percent >=1)
        {
            Percent =1;
        } 
    }
    
    
    
    uint32_t Duty_Clock = (uint32_t)((pow(2,13)-1)*Percent);
    //printf("Duty Clock:%ld\n",Duty_Clock);
    ledc_set_duty(LEDC_MODE, Channel_Config, Duty_Clock);
    ledc_update_duty(LEDC_MODE, Channel_Config);
}
