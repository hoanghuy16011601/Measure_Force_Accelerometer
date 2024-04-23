#include "HUY_I2C_Drive.h"
#include "HUY_MPU6050_Drive.h"
#include <stdio.h>


#define Sample_Rate_Reg 0x19
#define Configuration_Reg 0x1A
#define Gyro_Config_Reg 0x1B
#define Accelerometer_Config_Reg 0x1C
#define INT_PIN_Reg 0x37
#define INT_Enable_Reg 0x38
#define Power_Management_1 0x6B
#define Who_Am_I 0x75


void MPU6050_Initialization(uint8_t I2C_Port,uint8_t MPU6050_Address)
{
    MPU6050_WriteValue(I2C_Port,MPU6050_Address,Power_Management_1,0x01);
    MPU6050_WriteValue(I2C_Port,MPU6050_Address,Sample_Rate_Reg,99u);
    MPU6050_WriteValue(I2C_Port,MPU6050_Address,Configuration_Reg,5u);
    MPU6050_WriteValue(I2C_Port,MPU6050_Address,Accelerometer_Config_Reg,0x10);
}

void MPU6050_WriteValue(uint8_t I2C_Port,uint8_t MPU6050_Address, uint8_t Register_Address, uint8_t Data)
{
    i2c_cmd_handle_t command;
    uint8_t Buffer[3] = {Register_Address,Data,NULL};
    uint8_t Queue[512] = {0};
    I2C_Create_Queue(&command,Queue,sizeof(Queue));
    I2C_Master_Start(command);
    I2C_Master_Connect_Device(command, MPU6050_Address,I2C_WRITE); /// Connect MPU6050
    I2C_Master_Write_bytes(command,Buffer);
    i2c_master_stop(command);
    I2C_Queue_Begin(I2C_Port,command);
    I2C_Queue_Delete(command);
}


void MPU6050_ReadValue(uint8_t I2C_Port,uint8_t MPU6050_Address, uint8_t Register_Address,uint8_t *Buffer_data)
{
    i2c_cmd_handle_t command;

    uint8_t Queue[512] = {0};
    I2C_Create_Queue(&command,Queue,sizeof(Queue));
    I2C_Master_Start(command);
    I2C_Master_Connect_Device(command, MPU6050_Address,I2C_WRITE); /// Connect MPU6050
    I2C_Master_Write_byte(command,Register_Address);
    I2C_Master_Start(command);
    I2C_Master_Connect_Device(command, MPU6050_Address,I2C_READ); /// Connect MPU6050
    I2C_Master_Read_Byte(command,Buffer_data);
    i2c_master_stop(command);
    I2C_Queue_Begin(I2C_Port,command);
    I2C_Queue_Delete(command);
}




