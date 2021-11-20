#include <RTC/DS3231.h>
#include <stdint.h>

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

void I2C_Finish_Communication() {
    UCB0CTLW0 |= UCTXSTP;           // Send stop
    while (UCB0CTLW0 & UCTXSTP);    // Wait for stop
}

void I2C_Send(uint8_t data) {
    UCB0TXBUF = data;               // Send value
    while (!(UCB0IFG & UCTXIFG0));  // Wait for tx interrupt
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

uint8_t I2C_Receive() {
    int8_t value = 0;
    while (!(UCB0IFG & UCRXIFG0));  // Wait for RX interrupt flag
    value = UCB0RXBUF;              // Read data
    return value;
}

uint8_t BCD_to_DEC(uint8_t value) {
    return value - 6 * (value >> 4);
}

uint8_t DEC_to_BCD(uint8_t value) {
    return value + 6 * (value / 10);
}

void I2C_Master_Mode_Init() {
    UCB0CTLW0 |= UCSWRST;                                // Software reset enabled (Puts eUSCI_B in configuration mode)
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC_1 | UCSSEL_2; // I2C mode, Master mode, Synchronous mode, SMCLK as clock source
    UCB0BRW = 10;                                        // Baud rate = SMCLK/10 = 100kHz
    UCB0CTLW0 &= ~UCSWRST;                               // Clear software reset
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

uint8_t* Get_Current_Time_and_Date(void) {
    static uint8_t date_and_time_array[7];
    int i = 0;

    static const uint8_t bit_mask[] = { 0x7F, 0x7F, 0x3F, 0x7, 0x3F, 0x1F, 0xFF };
    /* Extracts the effective data from the register,
     * removing not relevant information from some bits. */

    I2C_Request_From(DS3231_SLAVE_ADDR, DS3231_REG_SECONDS, 7);

    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // seconds
    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // minutes
    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // hours
    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // week day
    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // month day
    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // month
    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // year

    return date_and_time_array;
}

void Set_Clock_and_Calendar(uint8_t second, uint8_t minute, uint8_t hour, uint8_t week_day, uint8_t month_day, uint8_t month, uint8_t year) {
    I2C_Begin_Transmission(DS3231_SLAVE_ADDR, DS3231_REG_SECONDS, 7);

    I2C_Send(DEC_to_BCD(second));    // seconds
    I2C_Send(DEC_to_BCD(minute));    // minutes
    I2C_Send(DEC_to_BCD(hour));      // hours
    I2C_Send(DEC_to_BCD(week_day));  // week day (1 = sunday ~ 7 = saturday)
    I2C_Send(DEC_to_BCD(month_day)); // month day (1 to 31)
    I2C_Send(DEC_to_BCD(month));     // month
    I2C_Send(DEC_to_BCD(year));      // year (0 to 99)

    uint8_t bit_state = I2C_Read_Single_Byte(DS3231_SLAVE_ADDR, DS3231_REG_STATUS_CTL);          // I2C_Receive the status register
    I2C_Write_Single_Byte(DS3231_SLAVE_ADDR, DS3231_REG_STATUS_CTL, bit_state & ~_BV(OSF_BIT));  // clear the Oscillator Stop Flag
}

float Get_Temperature() {
    static const int16_t bit_mask = 0x00C0;
    /* Extracts the effective data from the register,
     * removing not relevant information from some bits. */

    int16_t MSB = I2C_Read_Single_Byte(DS3231_SLAVE_ADDR, DS3231_REG_TEMP_MSB) << 8;
    int16_t LSB = I2C_Read_Single_Byte(DS3231_SLAVE_ADDR, DS3231_REG_TEMP_LSB) & bit_mask;

    return (MSB | LSB) / 256.0; // Celsius degrees
}
