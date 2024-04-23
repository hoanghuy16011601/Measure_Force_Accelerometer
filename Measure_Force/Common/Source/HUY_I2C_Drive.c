#include <stdio.h>
#include <string.h>
#include "HUY_I2C_Drive.h"



void I2C_Master_Initialization(uint8_t I2C_Port ,uint8_t SLC_PIN, uint8_t SDA_PIN,uint32_t I2C_Frequency)
{
    i2c_config_t Custom_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num =SDA_PIN,         // select SDA GPIO specific
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_io_num = SLC_PIN,         // select SCL GPIO specific
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_Frequency,  
    .clk_flags = 0,                 
    };
    i2c_param_config(I2C_Port, &Custom_config);

    i2c_driver_install(I2C_Port, Custom_config.mode, 0, 0, 0);
}

void I2C_Create_Queue(i2c_cmd_handle_t *command,uint8_t *Buffer, uint32_t Size_Buffer)
{
    *command = i2c_cmd_link_create_static(Buffer,Size_Buffer);
}

void I2C_Queue_Begin(uint8_t I2C_Port,i2c_cmd_handle_t command)
{
    i2c_master_cmd_begin(I2C_Port, command, 1000/portTICK_PERIOD_MS);
}

void I2C_Queue_Delete(i2c_cmd_handle_t command)
{
    i2c_cmd_link_delete_static(command);
}

void I2C_Master_Start(i2c_cmd_handle_t command)
{
    i2c_master_start(command);
}
void I2C_Master_Connect_Device(i2c_cmd_handle_t command,uint8_t Address_Device,I2C_read_write_t Mode)
{
    i2c_master_write_byte(command, Address_Device<<1|Mode, ACK_CHECK_EN);

}

void I2C_Master_Write_byte(i2c_cmd_handle_t command, uint8_t Data)
{
    i2c_master_write_byte(command, Data, ACK_CHECK_EN);
}

void I2C_Master_Write_bytes(i2c_cmd_handle_t command,uint8_t *Buffer_Transmit)
{
    int Length = strlen((const char*) Buffer_Transmit);
    for (int i = 0; i < Length; i++)
    {
        i2c_master_write_byte(command, *(Buffer_Transmit+i), ACK_CHECK_EN);
    }
}

void I2C_Master_Read_Byte(i2c_cmd_handle_t command, uint8_t *Address_Data)
{
    i2c_master_read_byte(command,Address_Data,NACK_VAL);
}

void I2C_Master_Stop(i2c_cmd_handle_t command)
{
    i2c_master_stop(command);
}
