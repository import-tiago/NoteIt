#include <msp430.h>
#include <peripherals.h>

int main(void) {

	Init_Watchdog();
	Init_Oscillator();
	Init_Pins();
	Init_GPIO_Interrupt();

	while (1) {

	}
}

#pragma vector=PORT2_VECTOR
__interrupt void ISR_GPIO_PORT2(void) {

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
