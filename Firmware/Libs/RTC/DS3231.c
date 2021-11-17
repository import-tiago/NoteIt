#include <RTC/DS3231.h>
#include <stdint.h>

// Functions -----------------------------------------------------------------------------------------
void I2C_Begin_Transmission(uint8_t reg_addr) {
    UCB0IE &= ~UCRXIE;              // Clear rx interrupt
    UCB0IE &= ~UCTXIE;              // Clear tx interrupt

    UCB0CTLW0 |= UCTXSTT;           // Send start
    while (!(UCB0IFG & UCTXIFG0));

    UCB0TXBUF = reg_addr;           // Send register address
    while (!(UCB0IFG & UCTXIFG0));
}

void I2C_End_Transmission() {
    UCB0CTLW0 |= UCTXSTP;           // Send stop
    while (UCB0CTLW0 & UCTXSTP);    // Wait for stop
}

void endRequest() {
    while (UCB0CTLW0 & UCTXSTP);    // Wait for stop
}

void I2C_Write(uint8_t value) {
    UCB0TXBUF = value;           // Send value
    while (!(UCB0IFG & UCTXIFG0));  // Wait for tx interrupt
}

void requestFrom(uint8_t reg_addr, uint8_t n_bytes) {
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

uint8_t read() {
    int8_t value = 0;
    while (!(UCB0IFG & UCRXIFG0));  // Wait for RX interrupt flag
    value = UCB0RXBUF;              // Read data

    return value;
}

uint8_t BCD_to_DEC(uint8_t value) {
    return value - 6 * (value >> 4);
}

uint8_t DEC_to_BCD(uint8_t value) {
    //return value + 6 * (value / 10);
    //return ((value / 10 * 16) + (value % 10));
    return ((value / 10) << 4) + (value % 10);
}

uint8_t roundUpToTen(uint8_t val) {
    // This function will round seconds up the nearest number evenly divisible by 10, except
    // in the case of 09, 19, 29, ... seconds in which it will add 11 to it. For example:
    // roundUpToTen(42) = 50
    // roundUpToTen(29) = 40

    if (val == 59) {
        return 10;
    }

    if (val >= 49) {
        return 0;
    }

    uint8_t rem = val % 10;

    if (rem == 9) {
        val += 1;
        rem = 0;
    }

    return val + (10 - rem);
}

unsigned char RxData[2] = { 0, 0 };
unsigned char TxData[2] = { 0, 0 };
unsigned char TxAddr[2] = { 0, 0 };

void I2C_Init(uint8_t slave_addr) {
    UCB0CTLW0 |= UCSWRST;                          // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 + UCMST + UCSYNC + UCTR; // I2C mode, Master mode, sync, transmitter
    UCB0CTLW0 |= UCSSEL_2;                         // SMCLK = 1MHz
    UCB0BRW = 0xA;                               // Baud rate = SMLK/10 = 100kHz
    UCB0I2CSA = slave_addr;                       // Slave address
    UCB0CTLW0 &= ~UCSWRST;                         // Clear reset
}

unsigned int read_register(unsigned int reg_addr) {
    /* msb */
    TxAddr[0] = reg_addr >> 8;
    /* lsb */
    TxAddr[1] = reg_addr & 0xFF;

    UCB0CTLW1 = UCASTP_1;
    UCB0TBCNT = 0x0002;
    UCB0CTL1 &= ~UCSWRST;

    /* start i2c I2C_Write */
    UCB0CTL1 |= UCTXSTT + UCTR;
    while (!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = TxAddr[0];
    while (!(UCB0IFG & UCTXIFG0));
    UCB0TXBUF = TxAddr[1];
    while (!(UCB0IFG & UCBCNTIFG));

    /* i2c read */
    UCB0CTL1 &= ~UCTR;
    UCB0CTL1 |= UCTXSTT;
    while (!(UCB0IFG & UCRXIFG0));
    RxData[0] = UCB0RXBUF;
    UCB0CTLW0 |= UCTXSTP;
    while (!(UCB0IFG & UCRXIFG0));
    RxData[1] = UCB0RXBUF;
    while (!(UCB0IFG & UCSTPIFG));
    UCB0CTL1 |= UCSWRST;

    return RxData[1] << 8 | RxData[0];
}

void Get_Current_Time_and_Date(void) {
    int i = 0;

    static const uint8_t timeMask[] =
            { 0x7F, 0x7F, 0x3F, 0x7, 0x3F, 0x1F, 0xFF }; //{seconds, minutes, hours, day, date, month, year}

    requestFrom(DS3231_REG_SECONDS, 7);

    date_and_time_array[i++] = BCD_to_DEC(read() & timeMask[i]); // seconds
    date_and_time_array[i++] = BCD_to_DEC(read() & timeMask[i]); // minutes
    date_and_time_array[i++] = BCD_to_DEC(read() & timeMask[i]); // hours
    date_and_time_array[i++] = BCD_to_DEC(read() & timeMask[i]); // day
    date_and_time_array[i++] = BCD_to_DEC(read() & timeMask[i]); // date
    date_and_time_array[i++] = BCD_to_DEC(read() & timeMask[i]); // month
    date_and_time_array[i++] = BCD_to_DEC(read() & timeMask[i]); // year

    endRequest();

    __no_operation();
}

void Set_Current_Time_and_Date(uint8_t second, uint8_t minute, uint8_t hour,
                               uint8_t dayOfWeek, uint8_t dayOfMonth,
                               uint8_t month, uint8_t year) {

    int i = 0;

    static const uint8_t timeMask[] =
            { 0x7F, 0x7F, 0x3F, 0x7, 0x3F, 0x1F, 0xFF }; //{seconds, minutes, hours, day, date, month, year}

    second = DEC_to_BCD(second);
    minute = DEC_to_BCD(minute);
    hour = DEC_to_BCD(hour);
    dayOfWeek = DEC_to_BCD(dayOfWeek);
    dayOfMonth = DEC_to_BCD(dayOfMonth);
    month = DEC_to_BCD(month);
    year = DEC_to_BCD(year);

    I2C_Begin_Transmission(DS3231_I2C_ADDRESS);

    I2C_Write(DS3231_REG_SECONDS);        // send register address
    I2C_Write(second); // set seconds
    I2C_Write(minute); // set minutes
    I2C_Write(hour); // set hours
    I2C_Write(dayOfWeek); // set day of week (1=Sunday, 7=Saturday)
    I2C_Write(dayOfMonth); // set date (1 to 31)
    I2C_Write(month); // set month
    I2C_Write(year); // set year (0 to 99)

    I2C_End_Transmission();
}

void DS3231ClearAlarm1Bits(void) {
    // Configure USCI_B0 for I2C mode - Sending
    UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK; // I2C mode, Master mode, sync, Sending, SMCLK
    UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
    UCB0CTLW1 |= UCASTP_2;					  // Auto stop
    UCB0TBCNT = 5;			// Auto stop count - 4 alarm registers + address reg
    UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
    UCB0CTL1 &= ~UCSWRST;					  // Clear reset
    UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
    UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

    // Clear interrupt bit
    UCB0CTLW0 |= UCTXSTT;				// Send start
    while (!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
    UCB0TXBUF = DS3231_REG_STATUS_CTL;	// Send register address
    while (!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
    UCB0TXBUF = 0x00;		// 0b00000000 - No OSF, No 32KHz, No BSY, A2F, A1F
    while (!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
    UCB0CTLW0 |= UCTXSTP;				// Send stop
    while (UCB0CTLW0 & UCTXSTP);			// Wait for stop
}

void DS3231SetAlarm1Plus10Sec(void) {

    // Get current time's seconds, add 10 seconds
    uint8_t seconds = date_and_time_array[0];
    if (seconds < 50) {
        seconds = seconds + 10;
    }
    else {
        seconds = seconds - 50;
    }
    seconds = DEC_to_BCD(seconds);	// Convert to BCD format

    // Configure USCI_B0 for I2C mode - Sending
    UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK; // I2C mode, Master mode, sync, Sending, SMCLK
    UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
    UCB0CTLW1 |= UCASTP_2;					  // Auto stop
    UCB0TBCNT = 0x6;// Auto stop count - 6 greater than necessary, will manual stop
    UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
    UCB0CTL1 &= ~UCSWRST;					  // Clear reset
    UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
    UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

    UCB0CTLW0 |= UCTXSTT;				// Send start
    while (!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt

    UCB0TXBUF = DS3231_REG_A1SECONDS;	// Send register address
    while (!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt

    UCB0TXBUF = seconds & 0x7F;	// Mask and send seconds
    while (!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt

    UCB0TXBUF = 0x80;				// Send A1M2 Mask
    while (!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt

    UCB0TXBUF = 0x80;				// Send A1M3 Mask
    while (!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt

    UCB0TXBUF = 0x80;				// Send A1M4 Mask
    while (!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt

    UCB0CTLW0 |= UCTXSTP;		    // Send stop
    while (UCB0CTLW0 & UCTXSTP);	// Wait for stop

    DS3231ClearAlarm1Bits();

}

void DS3231SetAlarm1Round10Sec(void) {
    // This function is very similar to DS3231SetAlarm1Plus10Sec().
    // If RAM or memory becomes a problem, they will be merged to reduce resource consumption

    // Get current time's seconds, round to semi-nearest 10 seconds
    uint8_t seconds = date_and_time_array[0];
    seconds = roundUpToTen(seconds);
    seconds = DEC_to_BCD(seconds);	// Convert to BCD format

    // Configure USCI_B0 for I2C mode - Sending
    UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK; // I2C mode, Master mode, sync, Sending, SMCLK
    UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
    UCB0CTLW1 |= UCASTP_2;					  // Auto stop
    UCB0TBCNT = 0x6;// Auto stop count - 6 greater than necessary, will manual stop
    UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
    UCB0CTL1 &= ~UCSWRST;					  // Clear reset
    UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
    UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

    UCB0CTLW0 |= UCTXSTT;					// Send start
    while (!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt
    UCB0TXBUF = DS3231_REG_A1SECONDS;		// Send register address
    while (!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt
    UCB0TXBUF = seconds & 0x7F;				// Mask and send seconds
    while (!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt
    UCB0TXBUF = 0x80;						// Send A1M2 Mask
    while (!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt
    UCB0TXBUF = 0x80;						// Send A1M3 Mask
    while (!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt
    UCB0TXBUF = 0x80;						// Send A1M4 Mask
    while (!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt
    UCB0CTLW0 |= UCTXSTP;					// Send stop
    while (UCB0CTLW0 & UCTXSTP);				// Wait for stop

    DS3231ClearAlarm1Bits();

}

void DS3231TurnAlarm1On(void) {
    // Configure USCI_B0 for I2C mode - Sending
    UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK; // I2C mode, Master mode, sync, Sending, SMCLK
    UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
    UCB0CTLW1 |= UCASTP_2;					  // Auto stop
    UCB0TBCNT = 2;			  				  // Address + 1 reg
    UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
    UCB0CTL1 &= ~UCSWRST;					  // Clear reset
    UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
    UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

    // Set interrupt bit
    UCB0CTLW0 |= UCTXSTT;				// Send start
    while (!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
    UCB0TXBUF = DS3231_REG_CONTROL;		// Send register address
    while (!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
    UCB0TXBUF = 0x1D;// 0b00011101 - Osc on, SQWV Off, No convert temp, RS2,1 Whatever, INTCN 1, A2IE off, A1IE on
    while (UCB0CTLW0 & UCTXSTP);			// Wait for stop
}

void DS3231TurnAlarm1Off(void) {
    // Configure USCI_B0 for I2C mode - Sending
    UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK; // I2C mode, Master mode, sync, Sending, SMCLK
    UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
    UCB0CTLW1 |= UCASTP_2;					  // Auto stop
    UCB0TBCNT = 2;			  				  // Address + 1 reg
    UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
    UCB0CTL1 &= ~UCSWRST;					  // Clear reset
    UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
    UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

    // Clear interrupt bit
    UCB0CTLW0 |= UCTXSTT;				// Send start
    while (!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
    UCB0TXBUF = DS3231_REG_CONTROL;		// Send register address
    while (!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
    UCB0TXBUF = 0x1C;// 0b00011101 - Osc on, SQWV Off, No convert temp, RS2,1 Whatever, INTCN 1, A2IE off, A1IE off
    while (UCB0CTLW0 & UCTXSTP);			// Wait for stop
}
