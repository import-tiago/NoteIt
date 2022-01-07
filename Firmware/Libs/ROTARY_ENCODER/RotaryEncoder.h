#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <stdint.h>

#define DEBOUNCE_TIME 3




static const int8_t _rotaryEncoderTable[] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };
int8_t test(int8_t _a, int8_t _b);
int8_t Rotary_Encoder_Read(void) ;
uint8_t Rotary_Encoder_Changed(void);
uint8_t Rotary_Encoder_Push_Button(void);

uint8_t Rotary_Encoder_is_Clockwise(void);
uint8_t Rotary_Encoder_is_Counterclockwise(void);

int8_t m_val;      ///< current increment value: 1 (CW), -1 (CCW), 0 (invalid)
uint8_t m_code;     ///< previous / next code pattern
uint16_t m_store;    ///< stored pattern

uint8_t buttonState;
uint8_t lastButtonState;
uint8_t buttonPushCounter;

#endif // ROTARY_ENCODER_H
