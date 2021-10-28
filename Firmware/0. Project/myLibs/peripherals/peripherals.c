///////////////////////////////////////////////////////////////////////////////
// Author: Tiago Silva
// E-mail: tiagodepaulasilva@gmail.com
// 2021 - São Paulo, Brazil
///////////////////////////////////////////////////////////////////////////////

//DEPENDENT LIBRARIES
#include <msp430.h>
#include <board/board.h>

//CONSTANTS
#define TA0_OVERFLOW 63394 // 1 second overflow

void Init_Watchdog(void) {
	WDTCTL = WDTPW + WDTHOLD;	// Stop watchdog timer
}

void Init_Oscillator(void) {

	// DCO as 1MHz (source oscillator)
	if (CALBC1_1MHZ == 0xFF) 	// If calibration constants erased
		while (1); 				// do not load, trap CPU!!

	DCOCTL = 0;					// Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_1MHZ;		// Set range
	DCOCTL = CALDCO_1MHZ;       // Set DCO step + modulation

	BCSCTL2 |= DIVS_1;

	//By default, MCLK and SMCLK (clock references) are connected to DCOCLK

}

void Init_Pins(void) {

	/*  FIRST STEP: All pins as GPIO, OUTPUT at LOW level (LPM Optimization)       */
	P1DIR = 0xFF;  //All pins as OUTPUT
	P1SEL = 0x00;  //All pins as GPIO
	P1OUT = 0x00;  //All pins as LOW

	P2DIR = 0xFF;  //All pins as OUTPUT
	P2SEL = 0x00;  //All pins as GPIO
	P2OUT = 0x00;  //All pins as LOW

	/*  SECOND STEP: INPUT PINS  */
	P2DIR &= ~GPIO_SDCARD_DETECT;
	P2DIR &= ~GPIO_ROTARY_ENCODER_SWITCH;
	P2DIR &= ~GPIO_ROTARY_ENCODER_SIGNAL_A;
	P2DIR &= ~GPIO_ROTARY_ENCODER_SIGNAL_B;

	/*  THIRD STEP: SET PULL-UPs and PULL-DOWNs  */
	P2REN |= GPIO_SDCARD_DETECT; 			// Sets pull-down

	P2OUT |= GPIO_ROTARY_ENCODER_SWITCH;
	P2REN |= GPIO_ROTARY_ENCODER_SWITCH; 	// Sets pull-up

	P2OUT |= GPIO_ROTARY_ENCODER_SIGNAL_A;
	P2REN |= GPIO_ROTARY_ENCODER_SIGNAL_A; // Sets pull-up

	P2OUT |= GPIO_ROTARY_ENCODER_SIGNAL_B;
	P2REN |= GPIO_ROTARY_ENCODER_SIGNAL_B; // Sets pull-up

	/*  FOURTH STEP: SPECIAL FUNCTIONS  */
	//  From Table 16 in data-sheet:
	P1SEL |= UART_TX_EXTERNAL_DEVICE_PIN;
	P1SEL2 |= UART_TX_EXTERNAL_DEVICE_PIN;

	//  From Table 19 in data-sheet:
	P1SEL |= SPI_CLK_PIN;
	P1SEL2 |= SPI_CLK_PIN;

	P1SEL |= SPI_MISO_PIN;
	P1SEL2 |= SPI_MISO_PIN;

	P1SEL |= SPI_MOSI_PIN;
	P1SEL2 |= SPI_MOSI_PIN;
}

void Init_GPIO_Interrupt(void) {

	P2IES &= ~GPIO_SDCARD_DETECT; // Select the wake-up edge trigger (0: low-to-high | 1: high-to-low)
	P2IE |= GPIO_SDCARD_DETECT;   		// Set the interrupt enable
	P2IFG &= ~GPIO_SDCARD_DETECT;			// Initialize clear

	P2IES |= GPIO_ROTARY_ENCODER_SWITCH; // Select the wake-up edge trigger (0: low-to-high | 1: high-to-low)
	P2IE |= GPIO_ROTARY_ENCODER_SWITCH;   // Set the interrupt enable
	P2IFG &= ~GPIO_ROTARY_ENCODER_SWITCH;	// Initialize clear

	P2IES |= GPIO_ROTARY_ENCODER_SIGNAL_A; // Select the wake-up edge trigger (0: low-to-high | 1: high-to-low)
	P2IE |= GPIO_ROTARY_ENCODER_SIGNAL_A; // Set the interrupt enable
	P2IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_A;	// Initialize clear

	P2IES |= GPIO_ROTARY_ENCODER_SIGNAL_B; // Select the wake-up edge trigger (0: low-to-high | 1: high-to-low)
	P2IE |= GPIO_ROTARY_ENCODER_SIGNAL_B; // Set the interrupt enable
	P2IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_B;	// Initialize clear

}
/*
 void Init_ADC() {
 ADCCTL0 &= ~ADCENC;    // Disable ADC conversion (needed for the next steps)
 ADCMCTL0 |= ADCSREF_3;            // VR+ = VeREF+ and VR– = AVSS
 ADCMCTL0 |= ADCINCH_11; // A11 as the highest channel for a sequence of conversions (goes down from A11)
 ADCCTL0 |= ADCSHT_10;        // ADC sample-and-hold time = 512 ADCCLK cycles
 ADCCTL0 |= ADCON;                // ADC on
 ADCCTL0 |= ADCMSC; // ADC sample-and-conversions are performed automatically as soon as the prior conversion is completed (sequential mode)
 ADCCTL1 |= ADCSHS_0;             // ADC sample-and-hold source = ADCSC bit
 ADCCTL1 |= ADCSHP; // ADC sample-and-hold pulse-mode select = SAMPCON signal is sourced from the sampling timer
 ADCCTL1 |= ADCCONSEQ_1; // ADC conversion sequence mode = Sequence-of-channels
 ADCCTL2 &= ~ADCRES; // ADC resolution = 12 bit (14 clock cycle conversion time)
 ADCCTL2 |= ADCRES_2; // ADC resolution = 12 bit (14 clock cycle conversion time)
 ADCIE |= ADCIE0;               // ADC interrupts enable
 }

 void Init_RTC() {
 RTCMOD = 1; // [(1024/32768) * 32] = 1s (RTC count re-load compare value at 32)
 RTCCTL |= RTCSS__XT1CLK; // Source = 32kHz crystal
 RTCCTL |= RTCSR;         // Clears the counter value
 RTCCTL |= RTCPS__1024;   // Divided by 1024
 RTCCTL |= RTCIE;         // Real-time interrupt enable
 }

 void Init_UART0() {
 UCA0CTLW0 |= UCSWRST;                     // Put eUSCI in reset
 UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK


 //UCA0BRW    = 8;
 //UCA0MCTLW |=  UCOS16 | UCBRF_10 | 0xF700;  //0xF700 is UCBRSx = 0xF7


 UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
 UCA0IE |= UCRXIE;                      // Enable USCI_A0 RX interrupt
 }

 void Init_UART1() {
 UCA1CTLW0 |= UCSWRST;                     // Put eUSCI in reset
 UCA1CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
 UCA1BRW = 8;
 UCA1MCTLW |= UCOS16 | UCBRF_10 | 0xF700;  //0xF700 is UCBRSx = 0xF7
 UCA1CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
 UCA1IE |= UCRXIE;                      // Enable USCI_A1 RX interrupt
 }

 void Write_UART0(char *sentence) {
 unsigned int size = strlen(sentence);
 unsigned int i = 0;

 while (size > 0) {
 UCA0TXBUF = sentence[i++];
 while (!(UCA0IFG & UCTXIFG));
 size--;
 }
 }

 void Write_UART1(char *sentence) {
 unsigned int size = strlen(sentence);
 unsigned int i = 0;

 while (size > 0) {
 UCA1TXBUF = sentence[i++];
 while (!(UCA1IFG & UCTXIFG));
 size--;
 }
 }
 */
void Init_Timer_A0(void) {

	TACCTL0 |= CCIE;                             // CCR0 interrupt enabled
	TACCR0 = TA0_OVERFLOW;
	TACTL |= TASSEL_2 | MC_1 | ID_3;            // SMCLK, up count mode, SMCLK/8
}

void Init_SPI(void) {
	UCB0CTL0 = UCCKPL + UCMSB + UCMST + UCSYNC;  // 3-pin, 8-bit SPI master
	UCB0CTL1 = UCSSEL_2 + UCSWRST;                     // SMCLK
	UCB0BR0 = 8;                          // /8
	UCB0BR1 = 0;                              //
	//UCB0MCTL = 0;                             // No modulation
	UCB0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**
	IFG2 &= ~UCB0RXIE;
	IE2 |= UCB0RXIE;                          // Enable USCI0 RX interrupt

	P2OUT &= ~SPI_SDCARD_CS_PIN;            // Now with SPI signals initialized,
	P2OUT |= SPI_SDCARD_CS_PIN;                            // reset slave

	__delay_cycles(75);                 // Wait for slave to initialize

}
