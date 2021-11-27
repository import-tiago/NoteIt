#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <stdint.h>

static const int8_t _rotaryEncoderTable[] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

int8_t Rotary_Encoder_Read() ;
uint8_t Rotary_Encoder_Changed(void);


uint8_t m_pinA;     ///< encoder pin A
uint8_t m_pinB;     ///< encoder pin B
int8_t m_val;      ///< current increment value: 1 (CW), -1 (CCW), 0 (invalid)
uint8_t m_code;     ///< previous / next code pattern
uint16_t m_store;    ///< stored pattern

#endif // ROTARY_ENCODER_H
