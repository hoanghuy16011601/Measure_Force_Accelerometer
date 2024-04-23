
#include <stdio.h>
#include "esp_log.h"
#include "HUY_GPIO_Drive.h"
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

Function_Callback_t Function_Callback = NULL;

static void IRAM_ATTR gpio_interrupt_handler(void * arg)
{
    int data = (int) arg;
    Function_Callback(data);
}

void Configured_Input_Mode(gpio_num_t GPIO_Number, Configure_Pullup Pullup, Configure_Pulldown Pulldown)
{
    gpio_config_t Custom_config = {};
    Custom_config.pin_bit_mask = (1<<GPIO_Number);
    Custom_config.intr_type = GPIO_INTR_DISABLE;
    Custom_config.mode = GPIO_MODE_INPUT;
    Custom_config.pull_up_en = Pullup;
    Custom_config.pull_down_en = Pulldown;
    gpio_config(&Custom_config);
}

int Read_GPIO(gpio_num_t GPIO_Number)
{
    return gpio_get_level(GPIO_Number);
}

void Configure_Output_Mode(gpio_num_t GPIO_Number)
{
    gpio_config_t Custom_config = {};
    Custom_config.pin_bit_mask = (1<<GPIO_Number);
    Custom_config.intr_type = GPIO_INTR_DISABLE;
    Custom_config.mode = GPIO_MODE_OUTPUT;
    Custom_config.pull_up_en = GPIO_PULLUP_DISABLE;
    Custom_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&Custom_config);
}

void Write_GPIO(gpio_num_t GPIO_Number, Level_GPIO Level)
{
    gpio_set_level(GPIO_Number,Level);
}


void Configure_Interrupt_GPIO(gpio_num_t GPIO_Number,Interrupt_Edge Edge,Configure_Pullup Pullup, Configure_Pulldown Pulldown)
{
    gpio_config_t Custom_config = {};
    Custom_config.pin_bit_mask = (1<<GPIO_Number);
    Custom_config.intr_type = Edge;
    Custom_config.mode = GPIO_MODE_INPUT;
    Custom_config.pull_up_en = Pullup;
    Custom_config.pull_down_en = Pulldown;
    gpio_config(&Custom_config);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_Number,gpio_interrupt_handler,(void *)GPIO_Number);
}

void Function_GetAdress (void *Address_Func)
{
    Function_Callback = Address_Func;
}

void GPIO_Reset(uint8_t GPIO_Number)
{
    gpio_reset_pin(GPIO_Number);
}