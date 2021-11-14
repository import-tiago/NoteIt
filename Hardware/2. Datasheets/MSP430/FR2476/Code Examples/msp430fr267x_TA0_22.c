/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//  MSP430FR267x Demo - Timer0_A3, Ultra-Low Power Pulse Accumulator
//
//   Description: Timer_A is used as ultra-low power pulse counter. In this
//   example TAR is offset 10 counts, which are accumulated on TA0CLK P1.0,
//   with the system in LPM4 - all internal clocks off. After 10 counts, TAR
//   will overflow requesting an interrupt, and wake up the system. Timer_A is
//   then reconfigured with SMCLK as clock source in up mode - And timer period
//   is ~1ms with a 75% duty cycle on P1.1 and 25% on P1.2.
//
//                MSP430FR2676
//         ---------------
//     /|\|               |
//      | |               |
//      --|RST            |
//        |               |
//        |     P1.1/TA0.1|--> CCR1 - 75% PWM
//        |     P1.2/TA0.2|--> CCR2 - 25% PWM
//
//   Longyu Fang
//   Texas Instruments Inc.
//   August 2018
//   Built with IAR Embedded Workbench v7.12.1 & Code Composer Studio v8.1.0
//******************************************************************************
#include <msp430.h>
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                     // Stop WDT

    // Configure GPIO
    P1DIR |= BIT1 | BIT2;                     // P1.1 and P1.2 output
    P1DIR |= BIT1 | BIT2;                     // P1.1 and P1.2 output
    P1SEL1 |= BIT1 | BIT2;                    // P1.1 and P1.2 options select
    P1SEL1 |= BIT0;                           // P1.0 selected as TA0CLK
    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure Timer_A
    TA0CTL = TASSEL_0 | MC_2 | TACLR | TAIE;  // TA0CLK(external clock source), count mode, clear TAR, enable interrupt
    TA0R = 0xFFFF - 10;                       // Offset until TAR overflow

    __bis_SR_register(LPM4_bits | GIE);       // Enter LPM4, enable interrupts
    __no_operation();                         // For debugger
    __bis_SR_register(LPM0_bits);             // SMCLK is required
}
// Timer0_A3 Interrupt Vector (TAIV) handler
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) TIMER0_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(TA0IV,TA0IV_TAIFG))
    {
        case TA0IV_NONE:
            break;                               // No interrupt
        case TA0IV_TACCR1:
            break;                               // CCR1
        case TA0IV_TACCR2:
            break;                               // CCR2
        case TA0IV_TAIFG:
            // overflow
            TA0CCR0 = 1000-1;                         // PWM Period
            TA0CCTL1 = OUTMOD_7;                      // CCR1 reset/set
            TA0CCR1 = 750;                            // CCR1 PWM duty cycle
            TA0CCTL2 = OUTMOD_7;                      // CCR2 reset/set
            TA0CCR2 = 250;                            // CCR2 PWM duty cycle
            TA0CTL = TASSEL__SMCLK | MC__UP | TACLR;  // SMCLK, up mode, clear TAR
            __bic_SR_register_on_exit(LPM4_bits);     // Exit LPM4
            break;
        default:
            break;
    }
}
