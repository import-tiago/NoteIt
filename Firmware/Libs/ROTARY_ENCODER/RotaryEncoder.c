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

uint8_t Rotary_Encoder_Changed(void) {
    return m_val != 0;
}

static int8_t incrementValue(void) {
    return m_val;
}

static uint8_t isCW(void) {
    return m_code == 0x07;
}

uint8_t isCCW(void) {
    return m_code == 0x0b;
}
