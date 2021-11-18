#include <RTC/DS3231.h>
#include <stdint.h>

#define _BV(bit) (1 << (bit))
#define OSF 7
#define RTC_STATUS 0x0F

// Functions -----------------------------------------------------------------------------------------
void I2C_Begin_Transmission(uint8_t reg_addr) {

    I2C_Init(DS3231_SLAVE_ADDR);

    UCB0CTLW0 |= UCTXSTT;           // Send start
    while (!(UCB0IFG & UCTXIFG0));

    UCB0TXBUF = reg_addr;           // Send register address
    while (!(UCB0IFG & UCTXIFG0));
}

void I2C_End_Transmission() {
    UCB0CTLW0 |= UCTXSTP;           // Send stop
    while (UCB0CTLW0 & UCTXSTP);    // Wait for stop
}

void I2C_End_Request() {
    while (UCB0CTLW0 & UCTXSTP);    // Wait for stop
}

void I2C_Write(uint8_t value) {
    UCB0TXBUF = value;           // Send value
    while (!(UCB0IFG & UCTXIFG0));  // Wait for tx interrupt
}

void I2C_Request_From(uint8_t reg_addr, uint8_t n_bytes) {
    UCB0CTLW0 |= UCSWRST;           // Software reset enabled
    UCB0CTLW1 |= UCASTP_2;          // Auto stop
    UCB0TBCNT = n_bytes + 1;            // Auto stop count
    UCB0CTLW0 &= ~UCSWRST;          // Clear reset
    UCB0IE &= ~UCRXIE;              // Clear rx interrupt
    UCB0IE &= ~UCTXIE;              // Clear tx interrupt

    UCB0CTLW0 |= UCTXSTT;           // Send start
    while (!(UCB0IFG & UCTXIFG0));

    UCB0TXBUF = reg_addr;           // Send register address
    while (!(UCB0IFG & UCTXIFG0));

    UCB0CTLW0 &= ~UCTR;             // Change to receive
    UCB0CTLW0 |= UCTXSTT;           // Send start
    while (UCB0CTLW0 & UCTXSTT);    // Wait for start
}

uint8_t I2C_Read() {
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

void I2C_Init(uint8_t slave_addr) {
    UCB0CTLW0 |= UCSWRST;                          // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 + UCMST + UCSYNC + UCTR; // I2C mode, Master mode, sync, transmitter, SMCLK
    UCB0BRW = 0xA;                              // Baud rate = SMCLK/10 = 100kHz
    UCB0CTLW1 |= UCASTP_2;                    // Auto stop
    UCB0TBCNT = 0xA; // Auto stop count - 6 greater than necessary, will manual stop

    UCB0I2CSA = slave_addr;                       // Slave address
    UCB0CTLW0 &= ~UCSWRST;                         // Clear reset
    UCB0IE &= ~UCRXIE;                        // Clear rx interrupt
    UCB0IE &= ~UCTXIE;                        // Clear tx interrupt
}

// Read a single byte from RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
int8_t readRTC(uint8_t addr) {

    int8_t data;

    I2C_Request_From(addr, 1);

    data = I2C_Read();

    I2C_End_Request();

    return data;
}

// Write a single byte to RTC RAM.
// Valid address range is 0x00 - 0xFF, no checking.
// Returns the I2C status (zero if successful).
void writeRTC(uint8_t addr, int8_t value) {

    I2C_Begin_Transmission(addr);

    I2C_Write(value);

    I2C_End_Transmission();
}

void Get_Current_Time_and_Date(void) {
    int i = 0;

    I2C_Init(DS3231_SLAVE_ADDR);

    static const uint8_t timeMask[] =
            { 0x7F, 0x7F, 0x3F, 0x7, 0x3F, 0x1F, 0xFF }; //{seconds, minutes, hours, day, date, month, year}

    I2C_Request_From(DS3231_REG_SECONDS, 7);

    date_and_time_array[i++] = BCD_to_DEC(I2C_Read() & timeMask[i]); // seconds
    date_and_time_array[i++] = BCD_to_DEC(I2C_Read() & timeMask[i]); // minutes
    date_and_time_array[i++] = BCD_to_DEC(I2C_Read() & timeMask[i]); // hours
    date_and_time_array[i++] = BCD_to_DEC(I2C_Read() & timeMask[i]); // day
    date_and_time_array[i++] = BCD_to_DEC(I2C_Read() & timeMask[i]); // date
    date_and_time_array[i++] = BCD_to_DEC(I2C_Read() & timeMask[i]); // month
    date_and_time_array[i++] = BCD_to_DEC(I2C_Read() & timeMask[i]); // year

    I2C_End_Request();

    __no_operation();
}

void Set_Clock_and_Calendar(uint8_t second, uint8_t minute, uint8_t hour,
                            uint8_t dayOfWeek, uint8_t dayOfMonth,
                            uint8_t month, uint8_t year) {

    static const uint8_t timeMask[] =
            { 0x7F, 0x7F, 0x3F, 0x7, 0x3F, 0x1F, 0xFF };
    //{seconds, minutes, hours, day, date, month, year}

    second = DEC_to_BCD(second);
    minute = DEC_to_BCD(minute);
    hour = DEC_to_BCD(hour);
    dayOfWeek = DEC_to_BCD(dayOfWeek);
    dayOfMonth = DEC_to_BCD(dayOfMonth);
    month = DEC_to_BCD(month);
    year = DEC_to_BCD(year);

    I2C_Begin_Transmission(DS3231_REG_SECONDS);

    //I2C_Write(DS3231_REG_SECONDS);        // send register address
    I2C_Write(second); // set seconds
    I2C_Write(minute); // set minutes
    I2C_Write(hour); // set hours
    I2C_Write(dayOfWeek); // set day of week (1=Sunday, 7=Saturday)
    I2C_Write(dayOfMonth); // set date (1 to 31)
    I2C_Write(month); // set month
    I2C_Write(year); // set year (0 to 99)

    I2C_End_Transmission();
    //I2C_Init(DS3231_SLAVE_ADDR);
    uint8_t s = readRTC(RTC_STATUS);        // I2C_Read the status register
    writeRTC(RTC_STATUS, s & ~_BV(OSF));  // clear the Oscillator Stop Flag
}
