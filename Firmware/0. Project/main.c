#include <msp430.h>
//#include <peripherals/peripherals.h>

int main(void) {

	Init_Watchdog();
//	Init_Oscillator();
	__bis_SR_register(GIE);
	//Init_Pins();
	//Init_GPIO_Interrupt();
	//Init_Timer_A0();
	//Init_SPI();


	while (1) {

	}
}

void delay(unsigned int n) {
	for (; n > 0; n--)
		__delay_cycles(1000); // 1 ms
}

/*
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
*/
