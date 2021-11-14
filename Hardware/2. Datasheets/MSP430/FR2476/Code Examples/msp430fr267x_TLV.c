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
//  MSP430FR267x Demo -  TLV, Read device ID
//
//  Description: Read device ID from TLV, compare with the fixed value.
//               If the data is equl to the fixed value. turn on LED P1.0.

//
//           MSP430FR2676
//         ---------------
//     /|\|               |
//      | |               |
//      --|RST            |
//        |               |
//        |               |
//        |          P1.0 |---> LED
//        |               |
//
//   Longyu Fang
//   Texas Instruments Inc.
//   August 2018
//   Built with IAR Embedded Workbench v7.12.1 & Code Composer Studio v8.1.0
//******************************************************************************
#include <msp430.h>

#define DEVICE_ID_H  *((unsigned char *)0x1A05)                 // Device ID High byte
                                                                // See device datasheet for TLV table memory mapping
#define DEVICE_ID_L  *((unsigned char *)0x1A04)                 // Device ID Low byte


int main(void)
{
   WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

   P1OUT &= ~BIT0;                         // Clear P1.0 output latch for a defined power-on state
   P1DIR |= BIT0;                          // Set P1.0 to output direction

   PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                           // to activate previously configured port settings

   if((DEVICE_ID_H==0x83)&&(DEVICE_ID_L==0x28))
   {
       P1OUT |= BIT0;                      // Turn on LED P1.0
	   
   }

    __bis_SR_register(LPM0_bits | GIE);                           // LPM0 with interrupts enabled
    __no_operation();                                             // Only for debugger
}
