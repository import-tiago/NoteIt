#include <msp430.h>

#define LED BIT0
#define BT BIT3

int main(void) {

  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

  P1DIR |= LED;
  P1DIR &= ~BT;
  P1OUT |= BT;
  P1REN |= BT;//pull up

  P1OUT |= LED;


  while(1) {

      if ( (P1IN & BT) )
          P1OUT &= ~LED;
      else
          P1OUT |= LED;

  }

}
