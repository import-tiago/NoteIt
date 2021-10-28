#include <msp430.h>
#include <peripherals/peripherals.h>
#include <sdcard/mmc.h>

unsigned long cardSize = 0;
unsigned char status = 1;
unsigned int timeout = 0;
int i = 0;

unsigned char buffer[128];

int main(void) {

	Init_Watchdog();
	Init_Oscillator();
	__bis_SR_register(GIE);
	//Init_Pins();
	//Init_GPIO_Interrupt();
	//Init_Timer_A0();
	//Init_SPI();




	 //Initialisation of the MMC/SD-card
	  while (status != 0)                       // if return in not NULL an error did occur and the
	                                            // MMC/SD-card will be initialized again
	  {
	    status = mmcInit();
	    timeout++;
	    if (timeout == 150)                      // Try 50 times till error
	    {

	    	__no_operation();
	      //printf ("No MMC/SD-card found!! %x\n", status);
	      break;
	    }
	  }

	  while ((mmcPing() != MMC_SUCCESS));      // Wait till card is inserted

	  // Read the Card Size from the CSD Register
	  cardSize =  mmcReadCardSize();

	// Clear Sectors on MMC
	  for (i = 0; i < 512; i++) buffer[i] = 0;
	  mmcWriteSector(0, buffer);                // write a 512 Byte big block beginning at the (aligned) adress

	  for (i = 0; i < 512; i++) buffer[i] = 0;
	  mmcWriteSector(1, buffer);                // write a 512 Byte big block beginning at the (aligned) adress

	  mmcReadSector(0, buffer);                 // read a size Byte big block beginning at the address.
	  for (i = 0; i < 512; i++) if(buffer[i] != 0) //P1OUT |= 0x01;

	  mmcReadSector(1, buffer);                 // read a size Byte big block beginning at the address.
	  for (i = 0; i < 512; i++) if(buffer[i] != 0)// P1OUT |= 0x02;


	// Write Data to MMC
	  for (i = 0; i < 512; i++) buffer[i] = i;
	  mmcWriteSector(0, buffer);                // write a 512 Byte big block beginning at the (aligned) adress

	  for (i = 0; i < 512; i++) buffer[i] = i+64;
	  mmcWriteSector(1, buffer);                // write a 512 Byte big block beginning at the (aligned) adress

	  mmcReadSector(0, buffer);                 // read a size Byte big block beginning at the address.
	  for (i = 0; i < 512; i++) if(buffer[i] != (unsigned char)i) //P1OUT |= 0x04;

	  mmcReadSector(1, buffer);                 // read a size Byte big block beginning at the address.
	  for (i = 0; i < 512; i++) if(buffer[i] != (unsigned char)(i+64)) //P1OUT |= 0x08;

	  for (i = 0; i < 512; i++)
	    mmcReadSector(i, buffer);               // read a size Byte big block beginning at the address.

	  mmcGoIdle();                              // set MMC in Idle mode






	while (1) {

	}
}

void delay(unsigned int n) {
	for (; n > 0; n--)
		__delay_cycles(1000); // 1 ms
}

#pragma vector=PORT2_VECTOR
__interrupt void ISR_GPIO_PORT_2(void) {

	if ((P2IFG & GPIO_SDCARD_DETECT)) {
		P2IFG &= ~GPIO_SDCARD_DETECT;

	}

	if ((P2IFG & GPIO_ROTARY_ENCODER_SWITCH)) {
		P2IFG &= ~GPIO_ROTARY_ENCODER_SWITCH;

	}

	if ((P2IFG & GPIO_ROTARY_ENCODER_SIGNAL_A)) {
		P2IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_A;

	}

	if ((P2IFG & GPIO_ROTARY_ENCODER_SIGNAL_B)) {
		P2IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_B;

	}

}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void ISR_Timer_A0(void) { // 1 second overflow

}

// Test for valid RX and TX character
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
	__no_operation();
	/*
  volatile unsigned int i;

  while (!(IFG2 & UCB0TXIFG));              // USCI_B0 TX buffer ready?

  if (UCB0RXBUF == SLV_Data)                // Test for correct character RX'd
    P1OUT |= BIT0;                          // If correct, light LED
  else
    P1OUT &= ~BIT0;                         // If incorrect, clear LED

  MST_Data++;                               // Increment master value
  SLV_Data++;                               // Increment expected slave value
  UCB0TXBUF = MST_Data;                     // Send next value

  __delay_cycles(50);                     // Add time between transmissions to
  */
}                                           // make sure slave can keep up

