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

#include <./SDCARD/ff.h>
#include <./SDCARD/diskio.h>

#include <./ROTARY_ENCODER/RotaryEncoder.h>

#include <./DISPLAY/oled.h>
#include <./DISPLAY/Screens/Screens.h>

/*
typedef enum {
    HOME_SCREEN = 0, BAUDRATE_SCREEN, LOG_SETTINGS_SCREEN, TIME_AND_DATE_SCREEN
} Screens_List;
*/

//uint32_t Baudrate_List[] = { 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 230400 };

uint32_t Current_Baudrate = 115200;

//Screens_List Current_Screen = HOME_SCREEN;
//Screens_List next_screen;

#define DATALOGGER_IDLE_STATE 0
#define DATALOGGER_RECEIVING_STATE 1

char Datalogger_States[2][30] = { { "waiting data..." }, { "saving data..." } };

uint8_t Current_Datalogger_State = DATALOGGER_IDLE_STATE;

// 'CHECKED_BUTTON', 10x10px
const unsigned char Bitmap_CHECKED_BUTTON[] = { 0x00, 0x00, 0x3f, 0x00, 0x5e, 0x80, 0x6d, 0x80, 0x73, 0x80, 0x73, 0x80, 0x6d, 0x80, 0x5e, 0x80, 0x3f, 0x00, 0x00, 0x00 };
// 'CHECK_BUTTON', 10x10px
const unsigned char Bitmap_CHECK_BUTTON[] = { 0x00, 0x00, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x00, 0x00 };

// 'CUREENT_PAGE2', 5x5px
const unsigned char current_page_bitmap[] = { 0xf8, 0x88, 0x88, 0x88, 0xf8 };

uint8_t oled_buf[WIDTH * HEIGHT / 8];
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
    // string_typer(temp_x, 0, array, font_size, 1000);
    temp_x2 = temp_x + 4;
    // temp_x2 *= (f_width + space_char);

    // convert_font_size(temp_x2, 0, 128, font_size);
    //  string_typer(temp_x + 6, 0, "C", font_size, 1000);
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

    sprintf(clock, "%02dh%02d", hr, min);

    SSD1306_string(x, y, clock, font_size, 0, oled_buf);

}

void Show_Calendar(uint8_t x, uint8_t y, uint8_t font_size) {

    char calendar[20] = { 0 };

    char day = 0;
    char month = 0;
    char year = 0;

    memset(calendar, '\0', 20);

    g_current_time_and_date = Get_Current_Time_and_Date();

    day = *(g_current_time_and_date + 4);
    month = *(g_current_time_and_date + 5);

    sprintf(calendar, "%02d/%02d", day, month);

    SSD1306_string(x, y, calendar, font_size, 0, oled_buf);

}

void Build_Status_Bar() {
    // SSD1306_clear(oled_buf);
    Show_Clock(0, 0, 12);
    Show_Calendar(50, 0, 12);
    SSD1306_bitmap(86, 2, Bat816, 16, 8, oled_buf);
    SSD1306_string(104, 0, "100%", 12, 0, oled_buf);
    // SSD1306_display(oled_buf);
}
/*
void Build_Screen(Screens_List selected_screen) {

    uint8_t x = 0, y = 0;
    SSD1306_clear(oled_buf);
    Build_Status_Bar();
    if (selected_screen == HOME_SCREEN) {
        //Show_Current_Baudrate();
        x = 53;
        y = 50;
        SSD1306_bitmap(x, y, current_page_bitmap, 5, 5, oled_buf);
        SSD1306_string(x + 8, y - 10, "...", 14, 0, oled_buf);

    }
    else if (selected_screen == BAUDRATE_SCREEN) {
        x = 53;
        y = 50;
        SSD1306_string(x, y - 10, ".\0", 14, 0, oled_buf);
        SSD1306_bitmap(x + 8, y, current_page_bitmap, 5, 5, oled_buf);
        SSD1306_string(x + 15, y - 10, "..", 14, 0, oled_buf);
    }
    else if (selected_screen == LOG_SETTINGS_SCREEN) {
        x = 53;
        y = 50;
        SSD1306_string(x, y - 10, "..", 14, 0, oled_buf);
        SSD1306_bitmap(x + 15, y, current_page_bitmap, 5, 5, oled_buf);
        SSD1306_string(x + 23, y - 10, ".\0", 14, 0, oled_buf);
    }
    else if (selected_screen == TIME_AND_DATE_SCREEN) {
        x = 53;
        y = 50;
        SSD1306_string(x, y - 10, "...", 14, 0, oled_buf);
        SSD1306_bitmap(x + 23, y, current_page_bitmap, 5, 5, oled_buf);

    }

    SSD1306_display(oled_buf);

    Current_Screen = selected_screen;

}
*/
void Show_Current_Baudrate() {
    char *state;
    if (Current_Datalogger_State == DATALOGGER_IDLE_STATE)
        state = &Datalogger_States[DATALOGGER_IDLE_STATE][0];

    char array[20] = { 0 };
    ltoa(Current_Baudrate, array, 10);

    uint8_t array_len = strlen(array);
    uint8_t i = 0;

    uint8_t x = 5;
    uint8_t y = 15;
    uint8_t step = 20;

    for (i = 0; i < array_len; i++)
        SSD1306_char1616(x + (step * i), y, array[i], oled_buf);

    SSD1306_string(24, y + 18, state, 12, 0, oled_buf);

    // SSD1306_display(oled_buf);
    __no_operation();
}
/*
 void Show_Baudrate_List(uint8_t) {

 uint8_t x = 5;
 uint8_t y = 15;
 uint8_t step = 20;
 uint8_t i = 0;
 char array[20] = { 0 };

 uint8_t len = strlen(Baudrate_List);

 for (i = 0; i < len; i++)
 SSD1306_char1616(x, y, Baudrate_List[i], oled_buf);

 ltoa(Current_Baudrate, array, 10);

 SSD1306_string(24, y + 18, state, 12, 0, oled_buf);

 // SSD1306_display(oled_buf);
 __no_operation();

 }
 */

void Checks_if_Secreen_Changes() {

}

void Run_SFM() { //State Finite Machine
    /*
    switch (Current_Screen) {

        case HOME_SCREEN: {

            Build_Screen(HOME_SCREEN);

            __no_operation();

            uint8_t adj_status = Rotary_Encoder_Push_Button();
              next_screen = HOME_SCREEN;
            do {

                Rotary_Encoder_Read();

                if (Rotary_Encoder_Changed()) {
                    if (Rotary_Encoder_is_Clockwise()) {
                        if (next_screen < TIME_AND_DATE_SCREEN)
                            next_screen++;
                        else
                            next_screen = HOME_SCREEN;

                        Build_Screen(next_screen);
                    }
                    if (Rotary_Encoder_is_Counterclockwise()) {
                        if (next_screen > HOME_SCREEN)
                            next_screen--;
                        else
                            next_screen = TIME_AND_DATE_SCREEN;
                        Build_Screen(next_screen);
                    }

                }

            }
            while (Current_Screen == HOME_SCREEN);

            Current_Screen = next_screen;

            break;
        }

        case BAUDRATE_SCREEN: {
            Build_Screen(BAUDRATE_SCREEN);

            __no_operation();

            uint8_t adj_status = Rotary_Encoder_Push_Button();
          next_screen = BAUDRATE_SCREEN;
            do {

                Rotary_Encoder_Read();

                if (Rotary_Encoder_Changed()) {
                    if (Rotary_Encoder_is_Clockwise()) {
                        if (next_screen < TIME_AND_DATE_SCREEN)
                            next_screen++;
                        else
                            next_screen = HOME_SCREEN;

                        Build_Screen(next_screen);
                    }
                    if (Rotary_Encoder_is_Counterclockwise()) {
                        if (next_screen > HOME_SCREEN)
                            next_screen--;
                        else
                            next_screen = TIME_AND_DATE_SCREEN;
                        Build_Screen(next_screen);
                    }

                }

            }
            while (Current_Screen == BAUDRATE_SCREEN);

            Current_Screen = next_screen;

            break;

        }

        case LOG_SETTINGS_SCREEN: {

            Build_Screen(LOG_SETTINGS_SCREEN);

            __no_operation();

            uint8_t adj_status = Rotary_Encoder_Push_Button();
         next_screen = LOG_SETTINGS_SCREEN;
            do {

                Rotary_Encoder_Read();

                if (Rotary_Encoder_Changed()) {
                    if (Rotary_Encoder_is_Clockwise()) {
                        if (next_screen < TIME_AND_DATE_SCREEN)
                            next_screen++;
                        else
                            next_screen = HOME_SCREEN;

                        Build_Screen(next_screen);
                    }
                    if (Rotary_Encoder_is_Counterclockwise()) {
                        if (next_screen > HOME_SCREEN)
                            next_screen--;
                        else
                            next_screen = TIME_AND_DATE_SCREEN;
                        Build_Screen(next_screen);
                    }

                }

            }
            while (Current_Screen == LOG_SETTINGS_SCREEN);

            Current_Screen = next_screen;

            break;
        }

        case TIME_AND_DATE_SCREEN: {

            Build_Screen(TIME_AND_DATE_SCREEN);

            __no_operation();

            uint8_t adj_status = Rotary_Encoder_Push_Button();
           next_screen = TIME_AND_DATE_SCREEN;
            do {

                Rotary_Encoder_Read();

                if (Rotary_Encoder_Changed()) {
                    if (Rotary_Encoder_is_Clockwise()) {
                        if (next_screen < TIME_AND_DATE_SCREEN)
                            next_screen++;
                        else
                            next_screen = HOME_SCREEN;

                        Build_Screen(next_screen);
                    }
                    if (Rotary_Encoder_is_Counterclockwise()) {
                        if (next_screen > HOME_SCREEN)
                            next_screen--;
                        else
                            next_screen = TIME_AND_DATE_SCREEN;
                        Build_Screen(next_screen);
                    }

                }

            }
            while (Current_Screen == TIME_AND_DATE_SCREEN);

            Current_Screen = next_screen;

            break;
        }

    }
    */
}

void Populate_Array(uint16_t *dest, uint16_t *origin, uint16_t n) {

    while (n-- > 0) {
        *dest = *origin;

        dest++;
        origin++;
    }

}

/*
 * Struct to organize every screen in the current application
 */
//ScreenName[number of elements in screen]
//[number of parameters in each element (element name and number )][number of navigable options in the specific element number]
struct ScreensStruct {
    uint8_t HomeScreen[3][2][1];
    uint8_t Baudrate_Screen[3][2][1];
    uint8_t Log_Settings_Screen[3][2][1];
    uint8_t Clock_and_Calendar_Screen[4][2][1];
} Screens = {
    {{STATUS_BAR}, {0}},
    {{CURRENT_BAUD_RATE}, {0}},
    {{CHANGE_SCREEN_BUTTON}, {NUMBER_OF_SCREENS}},

    {{STATUS_BAR}, {0}},
    {{BAUD_RATE_SELECTION}, {BAUDRATE_LIST_LENGTH}},
    {{CHANGE_SCREEN_BUTTON}, {NUMBER_OF_SCREENS}},

    {{STATUS_BAR}, {0}},
    {{LOG_VARIABLES_SELECTION}, {0}},
    {{CHANGE_SCREEN_BUTTON}, {NUMBER_OF_SCREENS}},

    {{STATUS_BAR}, {0}},
    {{CLOCK_ADJUSTMENT}, {0}},
    {{CALENDAR_ADJUSTMENT}, {0}},
    {{CHANGE_SCREEN_BUTTON}, {NUMBER_OF_SCREENS}}
  };









/*
struct _Available_TCs {
    uint16_t Range[NUMBER_OF_TCs];
} Struct_TC_Ranges;

struct _Calibration_Points {
    uint16_t Point[NUMBER_OF_CALIBRATION_POINTS];
} Struct_Calibration_Points;

struct CalibrationProcess {
    struct _Available_TCs TC;
    struct _Calibration_Points Calibration_Points;

} Calibration;
*/

int main(void) {

    Watchdog_Init();
    GPIOs_Init();
    // GPIO_Interrupt_Init();
    //Oscillator_Init(); //16MHz
    SPI_Master_Mode_Init(eUSCI_A0); //SDCARD
    SPI_Master_Mode_Init(eUSCI_B1); //Display OLED
    I2C_Master_Mode_Init(eUSCI_B0); //RTC

    __enable_interrupt();

  //  Populate_Array((uint16_t*) &Calibration.TC.Range, (uint16_t*) &Available_TCs, NUMBER_OF_TCs);

    //Set_Clock_and_Calendar(0, 58, 13, 1, 28, 11, 21);

    SSD1306_begin();
    SSD1306_clear(oled_buf);

    while (1) {
        Run_SFM();
    }

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
        //P4OUT |= BIT6;
        while (1);
    }

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
            //P4OUT |= BIT6;
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

    while (1);

}

//EXTERNAL INPUT EDGE DETECT
#pragma vector=PORT4_VECTOR
__interrupt void ISR_Rotary_Encoder_Monitor(void) {

}
