#include "RotaryEncoder.h"
#include <stdint.h>
#include <./HAL_BOARD/HAL_BOARD.h>
#include <./HAL_MCU/HAL_MCU.h>

int8_t Rotary_Encoder_Read() {
    m_code <<= 2;
    if ((P4IN & GPIO_ROTARY_ENCODER_SIGNAL_A)) {
        m_code |= 0x01;
    }
    if ((P4IN & GPIO_ROTARY_ENCODER_SIGNAL_B)) {
        m_code |= 0x02;
    }
    m_code &= 0x0f;
    // If valid then store as 16 bit data.
    if (_rotaryEncoderTable[m_code]) {
        m_store <<= 4;
        m_store |= m_code;
        if ((m_store & 0xff) == 0x2b) {
            m_val = -1;
            return m_val;
        }
        if ((m_store & 0xff) == 0x17) {
            m_val = 1;
            return m_val;
        }
    }
    m_val = 0;
    return m_val;
}

int8_t test(int8_t _a, int8_t _b) {
    m_code <<= 2;

        m_code |= _a;

        m_code |= _b;

    m_code &= 0x0f;
    // If valid then store as 16 bit data.
    if (_rotaryEncoderTable[m_code]) {
        m_store <<= 4;
        m_store |= m_code;
        if ((m_store & 0xff) == 0x2b) {
            m_val = -1;
            return m_val;
        }
        if ((m_store & 0xff) == 0x17) {
            m_val = 1;
            return m_val;
        }
    }
    m_val = 0;
    return m_val;
}

uint8_t Rotary_Encoder_Push_Button(){

      buttonState = (P4IN & GPIO_ROTARY_ENCODER_BUTTON);

      if (buttonState != lastButtonState) {

        if (buttonState) {      // if the state has changed, increment the counter
          buttonPushCounter++; // if the current state is HIGH then the button went from off to on:
        } else {

        }
      }
      lastButtonState = buttonState;   // save the current state as the last state, for next time through the loop


return buttonPushCounter;

}

uint8_t Rotary_Encoder_Changed(void) {
    return m_val != 0;
}

static int8_t incrementValue(void) {
    return m_val;
}

 uint8_t Rotary_Encoder_is_Clockwise(void) {
    return m_code == 0x07;
}

uint8_t Rotary_Encoder_is_Counterclockwise(void) {
    return m_code == 0x0b;
}
