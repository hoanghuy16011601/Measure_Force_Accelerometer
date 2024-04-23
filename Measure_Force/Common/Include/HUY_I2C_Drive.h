#ifndef HUY_I2C_Drive_h
#define HUY_I2C_Drive_h

#include "driver/i2c.h"
#include <stdint.h>

#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */


typedef enum{
    I2C_WRITE =0,
    I2C_READ =1,
}I2C_read_write_t;


void I2C_Master_Initialization(uint8_t I2C_Port ,uint8_t SLC_PIN, uint8_t SDA_PIN,uint32_t I2C_Frequency);
void I2C_Create_Queue(i2c_cmd_handle_t *command,uint8_t *Buffer, uint32_t Size_Buffer);
void I2C_Queue_Begin(uint8_t I2C_Port,i2c_cmd_handle_t command);
void I2C_Queue_Delete(i2c_cmd_handle_t command);
void I2C_Master_Start(i2c_cmd_handle_t command);
void I2C_Master_Connect_Device(i2c_cmd_handle_t command,uint8_t Address_Device,I2C_read_write_t Mode);
void I2C_Master_Write_byte(i2c_cmd_handle_t command, uint8_t Data);
void I2C_Master_Write_bytes(i2c_cmd_handle_t command,uint8_t *Buffer_Transmit);
void I2C_Master_Read_Byte(i2c_cmd_handle_t command, uint8_t *Address_Data);
void I2C_Master_Stop(i2c_cmd_handle_t command);
#endif