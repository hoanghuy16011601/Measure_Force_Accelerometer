#ifndef HUY_GPIO_Drive_h
#define HUY_GPIO_Drive_h

#include "driver/gpio.h"

typedef enum{
    HIGH =1,
    LOW =0,
}Level_GPIO;

typedef enum{
    Pullup_Disable =0,
    Pullup_Enable =1,
}Configure_Pullup;

typedef enum{
    Pulldown_Disable =0,
    Pulldown_Enable =1,
}Configure_Pulldown;

typedef enum{
    Rising_Edge = 1,
    Falling_Edge =2,
    Any_Edge =3,
}Interrupt_Edge;

typedef void(*Function_Callback_t)(int);

void Configured_Input_Mode(gpio_num_t GPIO_Number, Configure_Pullup Pullup, Configure_Pulldown Pulldown);
int Read_GPIO(gpio_num_t GPIO_Number);
void Configure_Output_Mode(gpio_num_t GPIO_Number);
void Write_GPIO(gpio_num_t GPIO_Number, Level_GPIO Level);
void Configure_Interrupt_GPIO(gpio_num_t GPIO_Number,Interrupt_Edge Edge,Configure_Pullup Pullup, Configure_Pulldown Pulldown);
void Function_GetAdress (void *Address_Func);
void GPIO_Reset(uint8_t GPIO_Number);
#endif