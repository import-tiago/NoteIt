#include <./HAL_MCU/I2C/I2C.h>
#include <./HAL_BOARD/HAL_BOARD.h>
#include <./HAL_MCU/HAL_MCU.h>
#include <stdint.h>
#include <msp430.h>

void I2C_Master_Mode_Init(uint8_t eUSCI) {
    switch (eUSCI) {
       case eUSCI_B0:
        UCB0CTLW0 |= UCSWRST;                                // Software reset enabled (Puts eUSCI in configuration mode)
        UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC_1 | UCSSEL_2; // I2C mode, Master mode, Synchronous mode, SMCLK as clock source
        UCB0BRW = 10;                                        // Clock prescaler = SMCLK/10 = 100kHz
        UCB0CTLW0 &= ~UCSWRST;                               // Clear software reset
    break;
    }
}

void I2C_Begin_Transmission(uint8_t slave_addr, uint8_t reg_addr, uint8_t n_bytes) {
    UCB0I2CSA = slave_addr;         // target chip Slave address
    UCB0CTLW0 |= UCSWRST;           // Software reset enabled (Puts eUSCI_B in configuration mode)
    UCB0CTLW1 |= UCASTP_2;          // Auto stop generates
    UCB0TBCNT = n_bytes + 2;        // Auto stop count
    UCB0CTLW0 &= ~UCSWRST;          // Clears software reset

    UCB0CTLW0 |= UCTR;              // Transmitter mode
    UCB0CTLW0 |= UCTXSTT;           // Send start
    while (!(UCB0IFG & UCTXIFG0));

    UCB0TXBUF = reg_addr;           // Send register address
    while (!(UCB0IFG & UCTXIFG0));
}

void I2C_Request_From(uint8_t slave_addr, uint8_t register_addr, uint8_t n_bytes) {
    UCB0I2CSA = slave_addr;         // target chip Slave address
    UCB0CTLW0 |= UCSWRST;           // Software reset enabled (Puts eUSCI_B in configuration mode)
    UCB0CTLW1 |= UCASTP_2;          // Auto stop generates
    UCB0TBCNT = n_bytes + 1;        // Auto stop count
    UCB0CTLW0 &= ~UCSWRST;          // Clears software reset

    UCB0CTLW0 |= UCTR;              // Transmitter mode
    UCB0CTLW0 |= UCTXSTT;           // Send start
    while (!(UCB0IFG & UCTXIFG0));

    UCB0TXBUF = register_addr;      // Send register address
    while (!(UCB0IFG & UCTXIFG0));

    UCB0CTLW0 &= ~UCTR;             // Receiver mode
    UCB0CTLW0 |= UCTXSTT;           // Send Repeated Start Condition
    while (UCB0CTLW0 & UCTXSTT);    // Wait for start
}

void I2C_Send(uint8_t data) {
    UCB0TXBUF = data;               // Send value
    while (!(UCB0IFG & UCTXIFG0));  // Wait for tx interrupt
}

uint8_t I2C_Receive() {
    int8_t value = 0;
    while (!(UCB0IFG & UCRXIFG0));  // Wait for RX interrupt flag
    value = UCB0RXBUF;              // Read data
    return value;
}

void I2C_Finish_Communication() {
    UCB0CTLW0 |= UCTXSTP;           // Send stop
    while (UCB0CTLW0 & UCTXSTP);    // Wait for stop
}

int8_t I2C_Read_Single_Byte(uint8_t slave_addr, uint8_t register_addr) {
    I2C_Request_From(slave_addr, register_addr, 1);
    int8_t data = I2C_Receive();
    I2C_Finish_Communication();
    return data;
}

void I2C_Write_Single_Byte(uint8_t slave_addr, uint8_t register_addr, int8_t data) {
    I2C_Begin_Transmission(slave_addr, register_addr, 1);
    I2C_Send(data);
    I2C_Finish_Communication();
}
