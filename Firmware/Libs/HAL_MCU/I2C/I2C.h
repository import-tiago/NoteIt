#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

void I2C_Master_Mode_Init(uint8_t eUSCI);
void I2C_Begin_Transmission(uint8_t slave_addr, uint8_t reg_addr, uint8_t n_bytes);
void I2C_Request_From(uint8_t slave_addr, uint8_t register_addr, uint8_t n_bytes);
void I2C_Send(uint8_t data);
uint8_t I2C_Receive(void);
void I2C_Finish_Communication(void);
int8_t I2C_Read_Single_Byte(uint8_t slave_addr, uint8_t register_addr);
void I2C_Write_Single_Byte(uint8_t slave_addr, uint8_t register_addr, int8_t data);

#endif //I2C_H_
