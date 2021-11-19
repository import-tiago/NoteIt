#include <msp430.h>
#include <RTC/DS3231.h>
#include <stdint.h>
#include "./FatFS/ff.h"
#include "./FatFS/diskio.h"

void Software_Trim();            // Software Trim to get the best DCOFTRIM value
#define MCLK_FREQ_MHZ 1                     // MCLK = 1MHz

FATFS sdVolume;     // FatFs work area needed for each volume
FIL logfile;        // File object needed for each open file
uint16_t fp;        // Used for sizeof

uint8_t status = 17;    // Status variable that should change if successful

float testFloat = 85432.123;    // Sample floating point number
int32_t printValue[2]; // Size 2 array that will hold the split float for printing

void FloatToPrint(float floatValue, int32_t splitValue[2]) {
    int32_t i32IntegerPart;
    int32_t i32FractionPart;

    i32IntegerPart = (int32_t) floatValue;
    i32FractionPart = (int32_t) (floatValue * 1000.0f);
    i32FractionPart = i32FractionPart - (1000 * i32IntegerPart);
    if (i32FractionPart < 0) {
        i32FractionPart *= -1;
    }

    splitValue[0] = i32IntegerPart;
    splitValue[1] = i32FractionPart;
}

void i2cSetReset(void) {
    // TODO: Fix so delay isn't needed
    __delay_cycles(200);    // 200 us wait for whatever transfer to finish
    UCB0CTLW0 |= UCSWRST;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;       // Stop WDT
/*
    __bis_SR_register(SCG0);                // Disable FLL
    CSCTL3 = SELREF__REFOCLK;               // Set REFO as FLL reference source
    CSCTL1 = DCOFTRIMEN_1 | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_0; // DCOFTRIM=3, DCO Range = 1MHz
    CSCTL2 = FLLD_0 + 30;                   // DCODIV = 1MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                // Enable FLL
    Software_Trim();             // Software Trim to get the best DCOFTRIM value
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                               // default DCODIV as MCLK and SMCLK source

    */

    // Configure GPIO

    /* I2C */
#define SDA BIT2
#define SCL BIT3
    P1SEL0 |= SDA | SCL;                            // I2C pins

    PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode to activate previously configured port settings

    // Enable interrupts
    //__bis_SR_register(GIE);
    __enable_interrupt();
    I2C_Master_Mode_Init();

    Set_Clock_and_Calendar(0, 17, 21, 4, 17, 11, 21);



    while (1) {
        uint8_t array[7];
        uint8_t y = 0;
        uint8_t *time = Get_Current_Time_and_Date();
        float temp = Get_Temperature();

        for ( y = 0; y < 7; y++ ) {
            array[y] = *(time++);

           }


        _delay_cycles(10000);
        __no_operation();
    }


/*
    // Mount the SD Card
    switch (f_mount(&sdVolume, "", 0)) {
    __no_operation();
case FR_OK:
    status = 42;
    break;
case FR_INVALID_DRIVE:
    status = 1;
    break;
case FR_DISK_ERR:
    status = 2;
    break;
case FR_NOT_READY:
    status = 3;
    break;
case FR_NO_FILESYSTEM:
    status = 4;
    break;
default:
    status = 5;
    break;
    }

    if (status != 42) {
        // Error has occurred
        P4OUT |= BIT6;
        while (1);
    }

//  DS3231GetCurrentTime();

    char filename[] = "LOG2_00.csv";
    FILINFO fno;
    FRESULT fr;
    uint8_t i;
    for (i = 0; i < 100; i++) {
        filename[5] = i / 10 + '0';
        filename[6] = i % 10 + '0';
        fr = f_stat(filename, &fno);
        __no_operation();
        if (fr == FR_OK) {
            __no_operation();
            continue;
        }
        else if (fr == FR_NO_FILE) {
            __no_operation();
            break;
        }
        else {
            __no_operation();
            // Error occurred
            P4OUT |= BIT6;
            //  P1OUT |= BIT0;
            while (1);
        }
    }

    // Initialize result variable
    UINT bw = 0;

    FloatToPrint(testFloat, printValue);

    // Open & write
    if (f_open(&logfile, filename, FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) { // Open file - If nonexistent, create
        f_lseek(&logfile, logfile.fsize); // Move forward by filesize; logfile.fsize+1 is not needed in this application
        for (i = 0; i < 10; i++) {
            f_printf(&logfile, "%ld.%ld\n", printValue[0], printValue[1]);
        }
        f_sync(&logfile);
        testFloat += 1205.57;
        FloatToPrint(testFloat, printValue);
        for (i = 0; i < 10; i++) {
            f_printf(&logfile, "%ld.%ld\n", printValue[0], printValue[1]);
        }
        f_close(&logfile);                          // Close the file
        if (bw == 11) {
            __no_operation();
            //    P1OUT |= BIT0;
        }
    }

    //   P1OUT |= BIT0;
    __no_operation();
*/

}
void Software_Trim() {
    unsigned int oldDcoTap = 0xffff;
    unsigned int newDcoTap = 0xffff;
    unsigned int newDcoDelta = 0xffff;
    unsigned int bestDcoDelta = 0xffff;
    unsigned int csCtl0Copy = 0;
    unsigned int csCtl1Copy = 0;
    unsigned int csCtl0Read = 0;
    unsigned int csCtl1Read = 0;
    unsigned int dcoFreqTrim = 3;
    unsigned char endLoop = 0;

    do {
        CSCTL0 = 0x100;                         // DCO Tap = 256
        do {
            CSCTL7 &= ~DCOFFG;                  // Clear DCO fault flag
        }
        while (CSCTL7 & DCOFFG);               // Test DCO fault flag

        __delay_cycles((unsigned int) 3000 * MCLK_FREQ_MHZ); // Wait FLL lock status (FLLUNLOCK) to be stable
                                                             // Suggest to wait 24 cycles of divided FLL reference clock
        while ((CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)) && ((CSCTL7 & DCOFFG) == 0));

        csCtl0Read = CSCTL0;                   // Read CSCTL0
        csCtl1Read = CSCTL1;                   // Read CSCTL1

        oldDcoTap = newDcoTap;               // Record DCOTAP value of last time
        newDcoTap = csCtl0Read & 0x01ff;       // Get DCOTAP value of this time
        dcoFreqTrim = (csCtl1Read & 0x0070) >> 4;       // Get DCOFTRIM value

        if (newDcoTap < 256)                    // DCOTAP < 256
                {
            newDcoDelta = 256 - newDcoTap; // Delta value between DCPTAP and 256
            if ((oldDcoTap != 0xffff) && (oldDcoTap >= 256)) // DCOTAP cross 256
                endLoop = 1;                   // Stop while loop
            else {
                dcoFreqTrim--;
                CSCTL1 = (csCtl1Read & (~DCOFTRIM)) | (dcoFreqTrim << 4);
            }
        }
        else                                   // DCOTAP >= 256
        {
            newDcoDelta = newDcoTap - 256; // Delta value between DCPTAP and 256
            if (oldDcoTap < 256)                // DCOTAP cross 256
                endLoop = 1;                   // Stop while loop
            else {
                dcoFreqTrim++;
                CSCTL1 = (csCtl1Read & (~DCOFTRIM)) | (dcoFreqTrim << 4);
            }
        }

        if (newDcoDelta < bestDcoDelta)         // Record DCOTAP closest to 256
                {
            csCtl0Copy = csCtl0Read;
            csCtl1Copy = csCtl1Read;
            bestDcoDelta = newDcoDelta;
        }

    }
    while (endLoop == 0);                      // Poll until endLoop == 1

    CSCTL0 = csCtl0Copy;                       // Reload locked DCOTAP
    CSCTL1 = csCtl1Copy;                       // Reload locked DCOFTRIM
    while (CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)); // Poll until FLL is locked
}

