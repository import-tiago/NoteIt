#include <msp430.h>

#define LED BIT0
#define BT BIT3

int main(void) {

  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

  P1DIR &= ~BIT3;
  P1DIR |= BIT0;                            // Set P1.0 to output direction

  P1REN |= BIT0;
  P1OUT |= BIT0;

  P1OUT |= BIT0;       // if P1.4 set, set P1.0

  while (1) {
      /*
    if ( (BIT3 & P1IN) )
        P1OUT |= BIT0;
    else
        P1OUT &= ~BIT0;                    // else reset
    */
  }

}
