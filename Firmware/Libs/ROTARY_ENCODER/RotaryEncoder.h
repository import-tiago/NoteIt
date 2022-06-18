#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <stdint.h>

#define MINIMAL_DEBOUNCE_TIME 4 // mili-seconds


#define SWITCH_HOLD_TIME 1000
#define BUTTON_PRESSED 0

static uint32_t Rotary_Encoder_Switch_Holding = 0;
static uint8_t Rotary_Encoder_Switch_Release = 0;
static uint32_t Rotary_Encoder_Push_Button_Count = 0;

int8_t Rotary_Encoder_Read(void) ;
uint8_t Rotary_Encoder_Changed(void);
uint8_t Rotary_Encoder_Push_Button(void);

uint8_t Rotary_Encoder_is_Clockwise(void);
uint8_t Rotary_Encoder_is_Counterclockwise(void);

#endif // ROTARY_ENCODER_H
