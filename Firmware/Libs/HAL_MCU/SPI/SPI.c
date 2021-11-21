#include <msp430.h>
#include <stdint.h>
#include <./HAL_MCU/SPI/SPI.h>
#include <./HAL_BOARD/HAL_BOARD.h>
#include <./HAL_MCU/HAL_MCU.h>

void SPI_Master_Mode_Init(uint8_t eUSCI) {
    switch (eUSCI) {
    case eUSCI_A0:
        UCA0CTLW0 |=  UCSWRST;    // Software reset enabled (Puts eUSCI in configuration mode)
        UCA0CTLW0 |=
                       UCMODE_0 |  // 3-pin SPI mode (without slave transmission control a.k.a. UCxSTE pin)
                       UCMST_1  |  // Master mode
                       UCSYNC_1 |  // Synchronous mode
                       UCCKPH_1 |  // Clock phase = Data is captured on the first UCLK edge and changed on the following edge
                       UCCKPL_1 |  // Clock polarity = LOW as inactive state
                       UCSSEL_2 |  // eUSCI clock source = SMCLK
                       UCMSB;      // Direction of the receive and transmit shift register is MSB first
        UCA0BRW = 4;             // Clock prescaler = SMCLK/64 = 250kHz
        UCA0CTLW0 &= ~UCSWRST;    // Clear software reset
        break;


    case eUSCI_B1:
        UCB1CTLW0 |=  UCSWRST;    // Software reset enabled (Puts eUSCI in configuration mode)
        UCB1CTLW0 |=
                       UCMODE_0 |  // 3-pin SPI mode
                       UCMST_1  |  // Master mode
                       UCSYNC_1 |  // Synchronous mode
                       UCCKPH_1 |  // Clock phase = Data is captured on the first UCLK edge and changed on the following edge
                       UCCKPL_1 |  // Clock polarity = LOW as inactive state
                       UCSSEL_2 |  // eUSCI clock source = SMCLK
                       UCMSB;      // Direction of the receive and transmit shift register is MSB first
        UCA1BRW = 4;             // Clock prescaler = SMCLK/64 = 250kHz
        UCB1CTLW0 &= ~UCSWRST;    // Clear software reset
        break;
    }
}





static void rcvr_spi_m(uint8_t *dst) {
    *dst = rcvr_spi();
}

// Receive a byte from MMC via SPI  (Platform dependent)
static uint8_t rcvr_spi(void) {
    uint8_t ui8RcvDat;				// Receive variable

    uint16_t gie = __get_SR_register() & GIE;	// Save interrupt state
    __disable_interrupt();

    UCA0IFG &= ~UCRXIFG;				// Ensure RXIFG clear
    while (!(UCA0IFG & UCTXIFG));			// Wait for TX ready
    UCA0TXBUF = 0xFF;				// Send dummy byte
    while (!(UCA0IFG & UCRXIFG));			// Wait for RX buffer

    ui8RcvDat = UCA0RXBUF;				// Read RX buffer

    __bis_SR_register(gie);				// Reload interrupt state

    return (uint8_t) ui8RcvDat;

}

// Transmit a byte to MMC via SPI  (Platform dependent)                 
static void xmit_spi(uint8_t dat) {
    uint16_t gie = __get_SR_register() & GIE;	// Save interrupt state
    __disable_interrupt();				// Disable interrupts

    while (!(UCA0IFG & UCTXIFG));			// Wait for TX ready
    UCA0TXBUF = dat;				// Write byte
    while (UCA0STATW & UCBUSY);

    UCA0RXBUF;					// Read to empty RX buffer, clear any ovverrun

    __bis_SR_register(gie);				// Reload interrupt state
}
