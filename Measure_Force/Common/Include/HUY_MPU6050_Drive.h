#ifndef HUY_MPU6050_Drive_H
#define HUY_MPU6050_Drive_H


void MPU6050_Initialization(uint8_t I2C_Port,uint8_t MPU6050_Address);
void MPU6050_WriteValue(uint8_t I2C_Port,uint8_t MPU6050_Address, uint8_t Register_Address, uint8_t Data);
void MPU6050_ReadValue(uint8_t I2C_Port,uint8_t MPU6050_Address, uint8_t Register_Address,uint8_t *Buffer_data);

#endif