#include "RotaryEncoder.h"
#include <stdint.h>
#include <./HAL_BOARD/HAL_BOARD.h>
#include <./HAL_MCU/HAL_MCU.h>

uint32_t current_debouncing_tick_ms = 0;
uint32_t initial_debouncing_time = 0;
static int16_t rotary_value = 0;
static int16_t last_rotary_value = 0;

int8_t first_enconder_clock = 1;

int8_t Rotary_Encoder_Read() {
    return rotary_value;
}

uint8_t Rotary_Encoder_Push_Button() {
    return (P4IN & GPIO_ROTARY_ENCODER_BUTTON);
}

uint8_t Rotary_Encoder_Changed(void) {
    return last_rotary_value != rotary_value ? 1 : 0;
}

uint8_t Rotary_Encoder_is_Clockwise(void) {
    uint8_t v = last_rotary_value < rotary_value ? 1 : 0;
    if (v)
        last_rotary_value = rotary_value;
    return v;
}

uint8_t Rotary_Encoder_is_Counterclockwise(void) {
    uint8_t v = last_rotary_value > rotary_value ? 1 : 0;
    if (v)
        last_rotary_value = rotary_value;
    return v;
}

// ISR TIMER
#pragma vector = TIMER0_A0_VECTOR
__interrupt void ISR_Rotary_Enconder_Debounce() {

    static uint8_t state_signal_a;
    static uint8_t state_signal_b;

    current_debouncing_tick_ms++;

    if (initial_debouncing_time && (current_debouncing_tick_ms - initial_debouncing_time) >= MINIMAL_DEBOUNCE_TIME) {

        last_rotary_value = rotary_value;

        state_signal_a = (P4IN & GPIO_ROTARY_ENCODER_SIGNAL_A);
        state_signal_b = (P4IN & GPIO_ROTARY_ENCODER_SIGNAL_B);

        if ((!state_signal_a && state_signal_b) || (state_signal_a && !state_signal_b))
            rotary_value++;
        else
            rotary_value--;

        P4IES ^= GPIO_ROTARY_ENCODER_SIGNAL_A;
        P4IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_A;
        P4IE |= GPIO_ROTARY_ENCODER_SIGNAL_A;

        initial_debouncing_time = 0;
    }
}

// ISR EXTERNAL INPUT EDGE DETECT
#pragma vector=PORT4_VECTOR
__interrupt void ISR_Rotary_Encoder_Monitor() {
    if ((P4IFG & GPIO_ROTARY_ENCODER_SIGNAL_A)) {
        initial_debouncing_time = current_debouncing_tick_ms;
        P4IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_A;
        P4IE &= ~GPIO_ROTARY_ENCODER_SIGNAL_A;
    }
}
