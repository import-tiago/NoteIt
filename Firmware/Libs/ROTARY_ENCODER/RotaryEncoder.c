#include "RotaryEncoder.h"
#include <stdint.h>
#include <./HAL_BOARD/HAL_BOARD.h>
#include <./HAL_MCU/HAL_MCU.h>

uint8_t sys_tick_ms = 0;
uint8_t debouncing = 0;
static int16_t rotary_value = 0;
static int16_t last_rotary_value = 0;

int8_t Rotary_Encoder_Read() {
    return rotary_value;
}

uint8_t Rotary_Encoder_Push_Button() {
    buttonState = (P4IN & GPIO_ROTARY_ENCODER_BUTTON);

    if (buttonState != lastButtonState) {

        if (buttonState) {      // if the state has changed, increment the counter
            buttonPushCounter++; // if the current state is HIGH then the button went from off to on:
        }
        else {

        }
    }
    lastButtonState = buttonState;   // save the current state as the last state, for next time through the loop

    return buttonPushCounter;
}

uint8_t Rotary_Encoder_Changed(void) {
    return last_rotary_value != rotary_value ? 1 : 0;
}

uint8_t Rotary_Encoder_is_Clockwise(void){
    return last_rotary_value < rotary_value ? 1 : 0;
}

uint8_t Rotary_Encoder_is_Counterclockwise(void){
    return last_rotary_value > rotary_value ? 1 : 0;
}

//TIMER
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A() {

    static uint8_t state_signal_a = 0;
    static uint8_t state_signal_b = 0;

    sys_tick_ms++;

    if (debouncing && (sys_tick_ms - debouncing) >= DEBOUNCE_TIME) {

        last_rotary_value = rotary_value;

        state_signal_a = ((P4IN & GPIO_ROTARY_ENCODER_SIGNAL_A));
        state_signal_b = ((P4IN & GPIO_ROTARY_ENCODER_SIGNAL_B));

        if ((!state_signal_a && state_signal_b) || (state_signal_a && !state_signal_b))
            rotary_value++;
        else
            rotary_value--;

        P4IES ^= GPIO_ROTARY_ENCODER_SIGNAL_A;
        P4IE |= GPIO_ROTARY_ENCODER_SIGNAL_A;
        P4IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_A;
        P4IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_B;
        debouncing = 0;
    }
}

//EXTERNAL INPUT EDGE DETECT
#pragma vector=PORT4_VECTOR
__interrupt void ISR_Rotary_Encoder_Monitor() {
    if ((P4IFG & GPIO_ROTARY_ENCODER_SIGNAL_A)) {
        debouncing = sys_tick_ms;
        P4IE &= ~GPIO_ROTARY_ENCODER_SIGNAL_A;
        P4IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_A;
    }
}
