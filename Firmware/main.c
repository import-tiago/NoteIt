#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <./HAL_BOARD/HAL_BOARD.h>
#include <./HAL_MCU/HAL_MCU.h>
#include <./HAL_MCU/I2C/I2C.h>
#include <./HAL_MCU/SPI/SPI.h>
#include <./RTC/DS3231.h>
#include <./DISPLAY/SSD1306.h>
#include <./SDCARD/ff.h>
#include <./SDCARD/diskio.h>

int8_t *g_current_time_and_date;
float temp = 0;

int pos;

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

int main(void) {

    Watchdog_Init();
    GPIOs_Init();
    GPIO_Interrupt_Init();
    //Oscillator_Init(); //16MHz
    SPI_Master_Mode_Init(eUSCI_A0); //SDCARD
    SPI_Master_Mode_Init(eUSCI_B1); //Display OLED
    I2C_Master_Mode_Init(eUSCI_B0); //RTC

    __enable_interrupt();

    Display_Init();

    string_typer(0, 0, "TESTE", 2, 1000);

    Set_Clock_and_Calendar(0, 9, 21, 4, 23, 11, 21);

    char array_temp[10] = { 0 };
    char clock[20] = { 0 };
    char sec = 0;
    char min = 0;
    char hr = 0;

    while (1) {
        memset(array_temp, '\0', 10);
        memset(clock, '\0', 20);

        g_current_time_and_date = Get_Current_Time_and_Date();
        temp = Get_Temperature();
        sprintf(array_temp, "%.2f", temp);

        sec = *(g_current_time_and_date + 0);
        min = *(g_current_time_and_date + 1);
        hr = *(g_current_time_and_date + 2);

        sprintf(clock, "%d:%d:%d", hr, min, sec);

        string_typer(0, 0, array_temp, 2, 1000);
        string_typer(0, 4, clock, 2, 1000);
        _delay_cycles(10000);
        __no_operation();
    }

    /*

     Set_Clock_and_Calendar(0, 17, 21, 4, 17, 11, 21);

     while (1) {
     g_current_time_and_date = Get_Current_Time_and_Date();
     temp = Get_Temperature();
     _delay_cycles(10000);
     __no_operation();
     }

     //SD CARD Test
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

//EXTERNAL INPUT EDGE DETECT
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void) {

  //  P4IE &= ~(GPIO_ROTARY_ENCODER_BUTTON + GPIO_ROTARY_ENCODER_SIGNAL_A + GPIO_ROTARY_ENCODER_SIGNAL_B);

    if ((P4IFG & GPIO_ROTARY_ENCODER_BUTTON)) {
        if ((P4IN & GPIO_ROTARY_ENCODER_BUTTON))
            pos++;
        else
            pos--;

        //P4IES ^= GPIO_ROTARY_ENCODER_BUTTON;   // Toggle the edge trigger
        P4IFG &= ~GPIO_ROTARY_ENCODER_BUTTON;  // Clear interrupt flag
    }

    if ((P4IFG & GPIO_ROTARY_ENCODER_SIGNAL_A)) {
        if ((P4IN & GPIO_ROTARY_ENCODER_SIGNAL_A))
            pos++;
        else
            pos--;

        //P4IES ^= GPIO_ROTARY_ENCODER_SIGNAL_A;   // Toggle the edge trigger
        P4IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_A;  // Clear interrupt flag
    }

    if ((P4IFG & GPIO_ROTARY_ENCODER_SIGNAL_B)) {
        if ((P4IN & GPIO_ROTARY_ENCODER_SIGNAL_B))
            pos++;
        else
            pos--;

       // P4IES ^= GPIO_ROTARY_ENCODER_SIGNAL_B;   // Toggle the edge trigger
        P4IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_B;  // Clear interrupt flag
    }
   // P4IE |= GPIO_ROTARY_ENCODER_BUTTON | GPIO_ROTARY_ENCODER_SIGNAL_A | GPIO_ROTARY_ENCODER_SIGNAL_B;
}
