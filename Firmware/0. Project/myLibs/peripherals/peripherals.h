///////////////////////////////////////////////////////////////////////////////
// Author: Tiago Silva
// E-mail: tiagodepaulasilva@gmail.com
// 2021 - São Paulo, Brazil
///////////////////////////////////////////////////////////////////////////////

#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

//DEPENDENT LIBRARIES
#include <msp430.h>
#include <board/board.h>

//FUNCTIONS PROTOTYPES
void Init_Watchdog(void);

void Init_Oscillator(void);

void Init_Pins(void);

void Init_GPIO_Interrupt(void);

void Init_Timer_A0(void);

void Init_SPI(void);

#endif  // _PERIPHERALS_H_
