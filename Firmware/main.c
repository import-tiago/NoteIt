#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <./HAL_BOARD/HAL_BOARD.h>
#include <./HAL_MCU/HAL_MCU.h>
#include <./HAL_MCU/I2C/I2C.h>
#include <./HAL_MCU/SPI/SPI.h>
#include <./RTC/DS3231.h>
#include <./DISPLAY/SSD1306.h>
#include <./SDCARD/ff.h>
#include <./SDCARD/diskio.h>
#include <./ROTARY_ENCODER/RotaryEncoder.h>

int pos;
int8_t *g_current_time_and_date;
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

void delay_ms(uint8_t v) {
    while (v--)
        __delay_cycles(1000);
}

char array[5] = { 0 };
int8_t value = 0;
int8_t incValue = 0;

int countA = 0, countB = 0, stateA, stateB; //Declare required variables
int _a, _b;

void Show_Temperature(uint8_t x, uint8_t y, uint8_t font_size) {
    int temp_x = x;
    int temp_x2 = y;

    sprintf(array, "%.0f", Get_Temperature());
    string_typer(temp_x, 0, array, font_size, 1000);
    temp_x2 = temp_x + 4;
    temp_x2 *= (f_width + space_char);

    convert_font_size(temp_x2, 0, 128, font_size);
    string_typer(temp_x + 6, 0, "C", font_size, 1000);
}

void Show_Clock(uint8_t x, uint8_t y, uint8_t font_size) {

    char clock[20] = { 0 };

    char sec = 0;
    char min = 0;
    char hr = 0;

    memset(clock, '\0', 20);

    g_current_time_and_date = Get_Current_Time_and_Date();

    sec = *(g_current_time_and_date + 0);
    min = *(g_current_time_and_date + 1);
    hr = *(g_current_time_and_date + 2);

    sprintf(clock, "%02d:%02d", hr, min);

    string_typer(x, y, clock, font_size, 1000);
}

const unsigned char ti_logo[] = {
//   0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24    25    26    27
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0x80, 0x00, 0x00, 0x80, 0x98, 0x08, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00,
//  28    29    30    31    32    33    34    35    36    37    38    39    40    41    42    43    44    45    46    47    48    49    50    51    52    53    54    55
  0x00, 0x10, 0x30, 0x70, 0xF0, 0xF0, 0xF0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0x61, 0x01, 0x00, 0xF0, 0x3F, 0x01, 0x00, 0xE0, 0xE1, 0xF9, 0xFF, 0xFF, 0xFF, 0xF0, 0x00,
//  56    57    58    59    60    61    62    63    64    65    66    67    68    69    70    71    72    73    74    75    76    77    78    79    80    81    82    83
  0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x0F, 0x07, 0x03, 0x03, 0x03, 0x07, 0x1F, 0x60, 0xE0, 0xE0, 0xE1, 0xE0, 0xE0, 0x61, 0x31, 0x1F, 0x0F, 0x07, 0x07, 0x03, 0x03, 0x00,
//  84    85    86    87    88    89    90    91    92    93    94    95    96    97    98    99   100   101   102   103   104   105   106   107   108   109   110   111
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int main(void) {

    Watchdog_Init();
    GPIOs_Init();
    // GPIO_Interrupt_Init();
    //Oscillator_Init(); //16MHz
    SPI_Master_Mode_Init(eUSCI_A0); //SDCARD
    SPI_Master_Mode_Init(eUSCI_B1); //Db isplay OLED
    I2C_Master_Mode_Init(eUSCI_B0); //RTC

    __enable_interrupt();

    Display_Init();

    Set_Clock_and_Calendar(58, 9, 21, 4, 23, 11, 21);

    OLED_Display_Clear();

    write_char(0, 0, '.', 2);



    string_typer(0, 5, " ", 2, 1000);







    Show_Clock(0, 0, 0);
    Show_Temperature(13, 0, 0);

    static i = 0;

    while (1) {

        uint8_t adj_hour = 0;
        uint8_t adj_min = 0;
        uint8_t adj_status = 0;

        adj_status = Rotary_Encoder_Push_Button();
        do {

            Rotary_Encoder_Read();
            if (Rotary_Encoder_Changed()) {
                if (Rotary_Encoder_is_Clockwise()) {
                    if (i < 24)
                        i++;
                    else
                        i = 0;
                }
                if (Rotary_Encoder_is_Counterclockwise()) {
                    if (i > 0)
                        i--;
                    else
                        i = 23;
                }
                fill_display(DISPLAY_PIXELS_WIDTH, DISPLAY_PIXELS_HEIGHT, 0x00);
                sprintf(array, "%d", i);
                string_typer(0, 0, array, 2, 1000);
            }

        }
        while (adj_status == Rotary_Encoder_Push_Button());

        fill_display(DISPLAY_PIXELS_WIDTH, DISPLAY_PIXELS_HEIGHT, 0x00);

        string_typer(0, 0, "END", 2, 1000);
        while (1) {

        }

    }

    while (1) {
        /*
         memset(array_temp, '\0', 10);
         memset(clock, '\0', 20);

         g_current_time_and_date = Get_Current_Time_and_Date();
         temp = Get_Temperature();
         sprintf(array_temp, "%.2f", temp);

         sec = *(g_current_time_and_date + 0);
         min = *(g_current_time_and_date + 1);
         hr = *(g_current_time_and_date + 2);

         sprintf(clock, "%d:%d:%d", hr, min, sec);

         fill_display(DISPLAY_PIXELS_WIDTH, DISPLAY_PIXELS_HEIGHT, 0x00);

         string_typer(0, 0, array_temp, 2, 1000);
         string_typer(0, 4, clock, 2, 1000);
         _delay_cycles(10000);
         __no_operation();
         */
    }

    /*
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
__interrupt void ISR_Rotary_Encoder_Monitor(void) {

}
