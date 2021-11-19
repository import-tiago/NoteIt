#include <RTC/DS3231.h>
#include <stdint.h>

void I2C_Begin_Transmission(uint8_t slave_addr, uint8_t reg_addr) {

    UCB0I2CSA = slave_addr;     // target chip Slave address
    while (UCB0CTLW0 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTLW0 |= UCTR;    // Transmitter mode
    UCB0CTLW0 |= UCTXSTT; // Send start
    while (!(UCB0IFG & UCTXIFG0));

    UCB0TXBUF = reg_addr;           // Send register address
    while (!(UCB0IFG & UCTXIFG0));
}

void I2C_Finish_Transmission() {
    UCB0CTLW0 |= UCTXSTP;           // Send stop
    while (UCB0CTLW0 & UCTXSTP);    // Wait for stop
}

void I2C_End_Request() {
    UCB0CTLW0 |= UCTXSTP;           // Send stop
    while (UCB0CTLW0 & UCTXSTP);    // Wait for stop
}

void I2C_Send(uint8_t value) {
    UCB0TXBUF = value;           // Send value
    while (!(UCB0IFG & UCTXIFG0));  // Wait for tx interrupt
}

void I2C_Request_From(uint8_t slave_addr, uint8_t register_addr) {

    UCB0I2CSA = slave_addr;                       // target chip Slave address

    UCB0CTLW0 |= UCTR;    // Transmitter mode
    UCB0CTLW0 |= UCTXSTT; // Send start
    while (!(UCB0IFG & UCTXIFG0));

    UCB0TXBUF = register_addr;           // Send register address
    while (!(UCB0IFG & UCTXIFG0));

    //TODO: Repeated Start Condition (?)
    UCB0CTLW0 &= ~UCTR;             // Receiver mode
    UCB0CTLW0 |= UCTXSTT;           // Send start
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
    UCB0CTLW0 |= UCSWRST;                          // Enable changes operations to some bits from some registers from USCI_B
    UCB0BRW = 0xA;                              // Clock prescaler setting = SMCLK/UCB0BRW = 1MHz/10 = 100kHz
    UCB0CTLW0 |= UCMODE_3 | UCSYNC_1 | UCMST;                              // | UCTR; // I2C mode, Synchronous mode, Master mode, SMCLK
    // Single-master system as default
    // Single-slave addresses as default
    // 7-bit slave address as default
    //UCB0CTLW1 |= UCASTP_2;                         // Auto stop
    //UCB0TBCNT  = 0xF;                              // Auto stop count greater than necessary = will manual stop

    UCB0CTLW0 &= ~UCSWRST;                         // Disable changes operations to some bits from some registers from USCI_B

    //UCB0IE    &= ~UCRXIE;                          // Clear Rx interrupt
    //UCB0IE    &= ~UCTXIE;                          // Clear Tx interrupt
    UCB0IE   |= UCRXIE;                          // Clear Rx interrupt
    UCB0IE   |= UCTXIE;                          // Clear Tx interrupt
}

// Read a single byte from RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
int8_t I2C_Read_Single_Byte(uint8_t slave_addr, uint8_t addr) {

    I2C_Request_From(slave_addr, addr);

    int8_t data = I2C_Receive();

    I2C_End_Request();

    return data;
}

// Write a single byte to RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
// Returns the I2C status (zero if successful).
void I2C_Write_Single_Byte(uint8_t slave_addr, uint8_t register_addr, int8_t data) {

    I2C_Begin_Transmission(slave_addr, register_addr);

    I2C_Send(data);

    I2C_Finish_Transmission();
}

uint8_t* Get_Current_Time_and_Date(void) {

    static uint8_t date_and_time_array[7]; //{seconds, minutes, hours, day, date, month, year}
    int i = 0;

    /* Extracts the effective data from the register, removing information
     from bits not relevant to the data in question. */
    static const uint8_t bit_mask[] = { 0x7F, 0x7F, 0x3F, 0x7, 0x3F, 0x1F, 0xFF };
    // { seconds, minutes, hours, week_day, month_day, month, year }

    I2C_Request_From(DS3231_SLAVE_ADDR, DS3231_REG_SECONDS);

    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // seconds
    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // minutes
    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // hours
    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // week day
    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // month day
    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // month
    date_and_time_array[i++] = BCD_to_DEC(I2C_Receive() & bit_mask[i]); // year

    I2C_End_Request();

    return date_and_time_array;
}

void Set_Clock_and_Calendar(uint8_t second, uint8_t minute, uint8_t hour, uint8_t week_day, uint8_t month_day, uint8_t month, uint8_t year) {

    I2C_Begin_Transmission(DS3231_SLAVE_ADDR, DS3231_REG_SECONDS);

    I2C_Send(DEC_to_BCD(second)); // set seconds
    I2C_Send(DEC_to_BCD(minute)); // set minutes
    I2C_Send(DEC_to_BCD(hour)); // set hours
    I2C_Send(DEC_to_BCD(week_day)); // set day of week (1=Sunday, 7=Saturday)
    I2C_Send(DEC_to_BCD(month_day)); // set date (1 to 31)
    I2C_Send(DEC_to_BCD(month)); // set month
    I2C_Send(DEC_to_BCD(year)); // set year (0 to 99)

    I2C_Finish_Transmission();

    uint8_t bit_state = I2C_Read_Single_Byte(DS3231_SLAVE_ADDR, DS3231_REG_STATUS_CTL);          // I2C_Receive the status register
    I2C_Write_Single_Byte(DS3231_SLAVE_ADDR, DS3231_REG_STATUS_CTL, bit_state & ~_BV(OSF_BIT));  // clear the Oscillator Stop Flag
}

// Returns the temperature in Celsius times four.
float Get_Temperature() {

    static const int8_t bit_mask = 0xC0; /* Extracts the effective data from the register, removing information
     from bits not relevant to the data in question. */

    int16_t MSB = I2C_Read_Single_Byte(DS3231_SLAVE_ADDR, DS3231_REG_TEMP_MSB) << 8;
    int16_t LSB = I2C_Read_Single_Byte(DS3231_SLAVE_ADDR, DS3231_REG_TEMP_LSB) & bit_mask;

    float f = (MSB | LSB) / 256.0;
    return f;
}
