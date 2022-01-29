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

#define TA0_OVERFLOW        16000 // 1ms

float map(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t analogRead() {
    ADCCTL0 |= ADCENC | ADCSC;  // Sampling and conversion start
    while (!(ADCIFG & ADCIFG0));
    return ADCMEM0;

}

//ADC
#define ADC_POSITIVE_VREF              ((float)3.30)
#define ADC_RESOLUTION                 12
#define ADC_STEPS                      (1 << (ADC_RESOLUTION))
#define ADC_RESOLUTION_IN_VOLTAGE      ((float)(ADC_POSITIVE_VREF/ADC_STEPS))
#define VALUE_SIGNAL_DC_VOLTAGE        ((int)(ADC_STEPS / 2))

#define RESISTOR_VOLTAGE_DIVIDER_R1           1.0F // 1M ohm
#define RESISTOR_VOLTAGE_DIVIDER_R2           1.0F // 1M ohm
#define RESISTOR_VOLTAGE_DIVIDER_ATTENUATION  (RESISTOR_VOLTAGE_DIVIDER_R2 / (RESISTOR_VOLTAGE_DIVIDER_R1 + RESISTOR_VOLTAGE_DIVIDER_R2))

// BATTERY
#define VOLTAGE_REGULATOR_OUTPUT ((float)3.30)
#define VOLTAGE_REGULATOR_DROPOUT ((float)0.238)
#define MAX_VOLTAGE_BATTERY ((float)4.25)
#define MIN_VOLTAGE_BATTERY (VOLTAGE_REGULATOR_OUTPUT + VOLTAGE_REGULATOR_DROPOUT)
#define ADC_BATTERY_SAMPLES 100
int ADC_Battery_Array[ADC_BATTERY_SAMPLES];
int ADC_Battery_Mean = 0;
float Battery_Voltage = 0;
char current_battery_voltage[10] = {
                                     0 };
char current_battery_percentage[10] = {
                                        0 };

long Moving_Average(int instantaneous_value, int *array_values, int array_len) {

    long average = 0;
    int i = 0;

    // Shifts the entire buffer and discards the oldest value
    for (i = array_len - 1; i > 0; i--)
        *(&array_values[i]) = *(&array_values[i - 1]);

    *(&array_values[0]) = instantaneous_value;

    for (i = 0; i < array_len; i++)
        average += (long) *((&array_values[i]));

    return (average / array_len);
}

float Get_Battery_Voltage() {
    static float stable = 5;

    ADC_Battery_Mean = Moving_Average(analogRead(), &ADC_Battery_Array[0], ADC_BATTERY_SAMPLES);

    Battery_Voltage = (ADC_RESOLUTION_IN_VOLTAGE * ADC_Battery_Mean) / RESISTOR_VOLTAGE_DIVIDER_ATTENUATION;

    if (!(P4IN & GPIO_USB_DETECT)) {
        if (stable > Battery_Voltage)
            stable = Battery_Voltage;
    }
    else
        stable = Battery_Voltage;

    return stable;
}

//uint32_t Baudrate_List[] = { 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 230400 };
//#define BAUDRATE_POSSIBILITIES ( sizeof(Baudrate_List) / sizeof(Baudrate_List[0]) )

//Baudrate_List
//Screens_List Current_Screen = HOME_SCREEN;
//Screens_List next_screen;

void delay(uint16_t n) {
    while (n--)
        __delay_cycles(16000);
}

#define DATALOGGER_IDLE_STATE 0
#define DATALOGGER_RECEIVING_STATE 1

char Datalogger_States[2][30] = {
                                  {
                                    "waiting data..." },
                                  {
                                    "storing data..." } };

char Log_Variables[4][30] = {
                              {
                                "Add Date" },
                              {
                                "Add Time" },
                              {
                                "Add Timestamp" },
                              {
                                "Add Temperature" } };

#define NUMBER_OF_LOG_VARIABLES ((sizeof(Log_Variables) / sizeof(Log_Variables[0]))-1)

uint8_t Current_Datalogger_State = DATALOGGER_IDLE_STATE;

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

char array[5] = {
                  0 };
int8_t value = 0;
int8_t incValue = 0;

int countA = 0, countB = 0, stateA, stateB; //Declare required variables
uint32_t sys_tick_ms = 0;

uint32_t blinky_page_button_count;
uint8_t blinky_page_button;

uint32_t blinky_left_cursor_count;
uint8_t blinky_left_cursor;

uint32_t blinky_right_cursor_count;
uint8_t blinky_right_cursor;

uint32_t blinky_datalogger_status_count;
uint8_t blinky_datalogger_status;

uint32_t blinky_battery_symbol_count;
uint8_t blinky_battery_symbol;

uint32_t redraw_delay;
int new = 1;

uint32_t battery_voltage_update_count = 0;

int8_t Last_Screen_Builded = -1;

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

    char clock[20] = {
                       0 };

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

    char calendar[20] = {
                          0 };

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
    Show_Calendar(48, 0, 12);

    if (blinky_battery_symbol)
        SSD1306_bitmap(84, 2, Bat816, 16, 8, oled_buf);

    SSD1306_string(102, 0, current_battery_percentage, 12, 0, oled_buf);
    // SSD1306_display(oled_buf);
}

void Show_Screen_Title(uint8_t current_selected_screen) {

    char title[128] = {
                        0 };
    uint8_t x;
    uint8_t font_size = 12;

    uint8_t len;

    switch (current_selected_screen) {

        case HOME_SCREEN: {
            len = sprintf(title, "BAUD-RATE SELECTION");
            x = (WIDTH - (len * (font_size / 2))) / 2;
            SSD1306_string(x, 0, title, font_size, 0, oled_buf);

            break;
        }

        case LOG_SETTINGS_SCREEN: {
            len = sprintf(title, "LOG FILE SETTINGS");
            x = (WIDTH - (len * (font_size / 2))) / 2;
            SSD1306_string(x, 0, title, font_size, 0, oled_buf);
            break;
        }
        case CLOCK_AND_CALENDAR_SCREEN: {
            len = sprintf(title, "CLOCK & DATE ADJUST");
            x = (WIDTH - (len * (font_size / 2))) / 2;
            SSD1306_string(x, 0, title, font_size, 0, oled_buf);
            break;
        }

    }

    // SSD1306_display(oled_buf);
}

void Show_Datalogger_State() {
    char *state;
    if (Current_Datalogger_State == DATALOGGER_IDLE_STATE)
        state = &Datalogger_States[DATALOGGER_IDLE_STATE][0];

    if (blinky_datalogger_status)
        SSD1306_string(24, 34, state, 12, 0, oled_buf);
}

void Build_Scroll_Bar(uint8_t number_items, uint8_t current_page) {
    char bar_size = SCROLL_BAR_MAX_SIZE / number_items;
    uint8_t x = 124;
    uint8_t y = 16;

    if (!current_page)
        SSD1306_bitmap(x, y, scrollbar_bitmap, 4, bar_size, oled_buf);
    else
        SSD1306_bitmap(x, y * 2, scrollbar_bitmap, 4, bar_size, oled_buf);
}

void Build_Clock_and_Calendar_Adj() {

    char clock[20] = {
                       0 };

    char calendar[20] = {
                          0 };

    char min = 0;
    char hr = 0;

    char day = 0;
    char month = 0;
    char year = 0;

    memset(clock, '\0', 20);

    g_current_time_and_date = Get_Current_Time_and_Date();

    min = *(g_current_time_and_date + 1);
    hr = *(g_current_time_and_date + 2);

    day = *(g_current_time_and_date + 4);
    month = *(g_current_time_and_date + 5);
    year = *(g_current_time_and_date + 6);

    sprintf(clock, "%02d:%02dh", hr, min);
    sprintf(calendar, "%02d/%02d/20%02d", day, month, year);
    __no_operation();

    int8_t x = 0, y = 54, i = 0;

    x = 25;
    y = 15;
    char *state = &clock;

    int cursor = 0;

    if (!cursor) {
        SSD1306_string(x - 13, y + 3, ">", 12, 0, oled_buf);
        cursor = 1;

    }

    SSD1306_string(x, y, state, 15, 0, oled_buf);
    SSD1306_string(x, y + 17, calendar, 15, 0, oled_buf);

}

void Show_Current_Baudrate() {

    char array[20] = {
                       0 };

    ltoa(Baudrate_List[Current_Baudrate_Index], array, 10);

    uint8_t len = strlen(array);

    uint8_t step = 17;
    uint8_t font_size = 12;
    uint8_t cursor_font_size = 12;
    uint8_t i = 0;

    uint8_t x = (WIDTH - ((len - 1) * font_size) + ((len - 4) * 15) - (((len - 1) * step) - step)) / 2;
    uint8_t y = 17;

    SSD1306_string(4, y + 3, "<", 12, 1, oled_buf);
    SSD1306_string(120, y + 3, ">", cursor_font_size, 1, oled_buf);

    // if (blinky_left_cursor)
    //  SSD1306_string(4, y + 3, "<", 12, 1, oled_buf);

    // if (blinky_right_cursor)
    //  SSD1306_string(120, y + 3, ">", cursor_font_size, 1, oled_buf);

    for (i = 0; i < len; i++, x += step)
        SSD1306_char1616(x, y, array[i], oled_buf);
}

void Build_Navigation_Buttons(uint8_t current_selected_screen) {

    int8_t x = 0, y = 54, i = 0;

    x = (WIDTH - (NUMBER_OF_SCREENS * 5)) / 2;

    for (i = NUMBER_OF_SCREENS - 1; i >= 0; i--) {

        if (i == current_selected_screen) {
            if (blinky_page_button)
                SSD1306_bitmap((x + (i * 10)), y, current_page_bitmap, 5, 5, oled_buf);
        }
        else
            SSD1306_string(x + (i * 10), y - 10, ".", 14, 0, oled_buf);
    }
}

void Build_Screen(uint8_t screen_element[][3][1], int8_t number_elements) {

    SSD1306_clear(oled_buf);

    do {
        switch (screen_element[number_elements][1][0]) {
            case STATUS_BAR:
                if (Current_Screen == CHANGING_SECREEN_MODE)
                    Show_Screen_Title(screen_element[0][0][0]);
                else
                    Build_Status_Bar();
                break;

            case CURRENT_BAUD_RATE:

                Show_Current_Baudrate();
                break;

            case DATALOGGER_STATE:

                Show_Datalogger_State();
                break;

            case SCREENS_NAVIGATION_BUTTONS:
                Build_Navigation_Buttons(screen_element[0][0][0]);
                break;

            case BAUD_RATE_SELECTION:
                __no_operation();
                break;
            case LOG_INSERT_TEMPERATURE:

                Build_List_Log_Variables();
                __no_operation();
                break;
            case LOG_INSERT_TIME:
                __no_operation();
                break;
            case LOG_INSERT_DATE:
                __no_operation();
                break;
            case LOG_INSERT_EPOCH_TIMESTAMP:
                __no_operation();
                break;
            case CLOCK_ADJUSTMENT:

                Build_Clock_and_Calendar_Adj();
                __no_operation();
                break;
            case CALENDAR_ADJUSTMENT:
                __no_operation();
                break;
            case NUMBER_SCREEN_ELEMENTS:
                __no_operation();
                break;
        }
        number_elements--;
    }
    while (number_elements >= 0);

    SSD1306_display(oled_buf);

    if (screen_element[0][0][0] != CHANGING_SECREEN_MODE)
        Last_Screen_Builded = screen_element[0][0][0];
}

void Init_Timer0() {
    TA0CCTL0 |= CCIE;                      // TACCR0 interrupt enabled
    TA0CCR0 = TA0_OVERFLOW; // 1ms
    TA0CTL |= TASSEL__SMCLK | MC__UP;    // SMCLK, up count mode

    TA1CCTL0 |= CCIE;                      // TACCR0 interrupt enabled
    TA1CCR0 = TA0_OVERFLOW; // 1ms
    TA1CTL |= TASSEL__SMCLK | MC__UP;    // SMCLK, up count mode
}

void print_rotary_state() {

    SSD1306_clear(oled_buf);
    __delay_cycles(1000);

    char v[10] = {
                   0 };

    //sprintf(v, "%i", Rotary_Encoder_Read());
    sprintf(v, "%i", Rotary_Encoder_is_Clockwise());

    SSD1306_string(40, 30, v, 14, 0, oled_buf);

    SSD1306_display(oled_buf);

    __no_operation();
}

void Build_List_Log_Variables() {

    int8_t x = 0, y = 54, i = 0, y_cursor;
    char *variable_name;
    char cursor_space = 8;
    char checkbutton_space = 20;

    x = 1;
    y = 20;
    y_cursor = y;

    for (i = (Page_in_Screen * MAX_OPTIONS_PER_PAGE) - 1; i >= ((Page_in_Screen - 1) * MAX_OPTIONS_PER_PAGE); i--) {

        variable_name = &Log_Variables[i][0];

        if (blinky_right_cursor) {
            if (Page_in_Screen == MAX_OPTIONS_PER_PAGE - 1)
                SSD1306_string(x, (y_cursor + 1) + (Current_Element_Selected * 16), ">", 12, 0, oled_buf);
            else
                SSD1306_string(x, (y_cursor + 1) + ((Current_Element_Selected - 2) * 16), ">", 12, 0, oled_buf);
        }

        int p = 0;

        if (i < MAX_OPTIONS_PER_PAGE) {

            p = (NUMBER_OF_LOG_VARIABLES + 1) - (abs(((NUMBER_OF_LOG_VARIABLES + 1)) - (Page_in_Screen * MAX_OPTIONS_PER_PAGE)) + 1);

            SSD1306_string(x + cursor_space, (abs(p - i) * 16) + y, variable_name, 12, 0, oled_buf);

            if (Screens.Log_Settings_Screen_Parameters[(i + 1)][2][0])
                SSD1306_bitmap(128 - checkbutton_space, (i * 16) + y + 2, Bitmap_CHECKED_BUTTON, 10, 10, oled_buf);
            else
                SSD1306_bitmap(128 - checkbutton_space, (i * 16) + y + 2, Bitmap_CHECK_BUTTON, 10, 10, oled_buf);
        }
        else {

            p = (NUMBER_OF_LOG_VARIABLES + 1) - (abs(((NUMBER_OF_LOG_VARIABLES + 1)) - (Page_in_Screen * MAX_OPTIONS_PER_PAGE)) + 1);

            SSD1306_string(x + cursor_space, (abs(p - i) * 16) + y, variable_name, 12, 0, oled_buf);

            if (Screens.Log_Settings_Screen_Parameters[i + 1][2][0])
                SSD1306_bitmap(128 - checkbutton_space, (abs(MAX_OPTIONS_PER_PAGE - i) * 16) + y + 2, Bitmap_CHECKED_BUTTON, 10, 10, oled_buf);
            else
                SSD1306_bitmap(128 - checkbutton_space, (abs(MAX_OPTIONS_PER_PAGE - i) * 16) + y + 2, Bitmap_CHECK_BUTTON, 10, 10, oled_buf);
        }
    }

    Build_Scroll_Bar(NUMBER_OF_LOG_VARIABLES, Page_in_Screen - 1);

}

void Run_SFM() { //State Finite Machine

    switch (Current_Screen) {

        case CHANGING_SECREEN_MODE: {

            uint8_t next_screen = Last_Screen_Builded;

            do {
                if (Rotary_Encoder_is_Clockwise()) {
                    if (Last_Screen < NUMBER_OF_SCREENS - 1) {
                        next_screen = ++Last_Screen;

                    }
                    else
                        next_screen = 0;

                    Last_Screen = next_screen;
                }
                else if (Rotary_Encoder_is_Counterclockwise()) {
                    if (Last_Screen > 0) {
                        next_screen = --Last_Screen;
                    }
                    else
                        next_screen = NUMBER_OF_SCREENS - 1;

                    Last_Screen = next_screen;
                }

                if (next_screen == HOME_SCREEN)
                    Build_Screen(Screens.Home_Screen_Parameters, Elements_in_Screen[next_screen]);

                else if (next_screen == LOG_SETTINGS_SCREEN)
                    Build_Screen(Screens.Log_Settings_Screen_Parameters, Elements_in_Screen[next_screen]);

                else if (next_screen == CLOCK_AND_CALENDAR_SCREEN)
                    Build_Screen(Screens.Clock_and_Calendar_Screen_Parameters, Elements_in_Screen[next_screen]);

                __no_operation();
            }
            while ((Rotary_Encoder_Push_Button() != BUTTON_PRESSED) || ((Rotary_Encoder_Push_Button() == BUTTON_PRESSED) && (Rotary_Encoder_Switch_Holding > SWITCH_HOLD_TIME)));

            Current_Screen = next_screen;

            break;
        }

        case HOME_SCREEN: {
            Build_Screen(Screens.Home_Screen_Parameters, Elements_in_Screen[Current_Screen]);
            if (Rotary_Encoder_is_Clockwise()) {
                if (Current_Baudrate_Index < BAUDRATE_LIST_LENGTH - 1)
                    Current_Baudrate_Index++;
                else
                    Current_Baudrate_Index = 0;
            }
            else if (Rotary_Encoder_is_Counterclockwise()) {
                if (Current_Baudrate_Index > 0)
                    Current_Baudrate_Index--;
                else
                    Current_Baudrate_Index = BAUDRATE_LIST_LENGTH - 1;

            }
            break;
        }

        case LOG_SETTINGS_SCREEN: {
            Build_Screen(Screens.Log_Settings_Screen_Parameters, Elements_in_Screen[Current_Screen]);
            if (Rotary_Encoder_is_Clockwise()) {
                if (Current_Element_Selected < Elements_in_Screen[Current_Screen] - MAX_OPTIONS_PER_PAGE) {
                    Current_Element_Selected++;

                    if (Current_Element_Selected >= MAX_OPTIONS_PER_PAGE) {
                        if (Page_in_Screen < MAX_OPTIONS_PER_PAGE)
                            Page_in_Screen++;
                    }
                }
            }
            else if (Rotary_Encoder_is_Counterclockwise()) {
                if (Current_Element_Selected > 0) {
                    Current_Element_Selected--;

                    if (Current_Element_Selected < MAX_OPTIONS_PER_PAGE) {
                        if (Page_in_Screen > MAX_OPTIONS_PER_PAGE - 1)
                            Page_in_Screen--;
                    }
                }
            }

            if (Rotary_Encoder_Push_Button() == BUTTON_PRESSED) {
                while (Rotary_Encoder_Push_Button() == BUTTON_PRESSED);
                Screens.Log_Settings_Screen_Parameters[Current_Element_Selected + 1][2][0] = !Screens.Log_Settings_Screen_Parameters[Current_Element_Selected + 1][2][0];

            }
            break;
        }

        case CLOCK_AND_CALENDAR_SCREEN: {
            Build_Screen(Screens.Clock_and_Calendar_Screen_Parameters, Elements_in_Screen[Current_Screen]);
            break;
        }

    }

}

int main(void) {

    Watchdog_Init();
    GPIOs_Init();
    Oscillator_Init(); //16MHz
    GPIO_Interrupt_Init();
    Init_Timer0();
    SPI_Master_Mode_Init(eUSCI_A0); //SDCARD
    SPI_Master_Mode_Init(eUSCI_B1); //Display OLED
    I2C_Master_Mode_Init(eUSCI_B0); //RTC
    ADC_Init();

    __enable_interrupt();

    SSD1306_begin();

    SSD1306_clear(oled_buf);
    SSD1306_display(oled_buf);

    int i = 0;
    for (i = 0; i < ADC_BATTERY_SAMPLES; i++)
        ADC_Battery_Array[i] = ADC_STEPS - 1;

    for (i = 0; i < ADC_BATTERY_SAMPLES; i++)
        Get_Battery_Voltage();

    sprintf(current_battery_percentage, "%.0f%%", map(Get_Battery_Voltage(), MIN_VOLTAGE_BATTERY, MAX_VOLTAGE_BATTERY, 0, 100));

    //SSD1306_string(20, 15, print_a, 14, 1, oled_buf);
    //SSD1306_display(oled_buf);
    //print_rotary_state();
    ///
    //  g_current_time_and_date = Get_Current_Time_and_Date();
    //  char min = *(g_current_time_and_date + 1);
    //   if(min <= 50)
    // Set_Clock_and_Calendar(0, 50, 14, SUNDAY, 2, 1, 22);
    //
    Build_Screen(Screens.Home_Screen_Parameters, Elements_in_Screen[HOME_SCREEN]);

    while (1) {
        __no_operation();
        Run_SFM();
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
     SSD1306_clear(oled_buf);
     SSD1306_display(oled_buf);
     __delay_cycles(1000);

     SSD1306_string(16, 20, "SD-CARD FAIL!", 16, 1, oled_buf);
     SSD1306_display(oled_buf);
     __delay_cycles(1000);
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
     */

}

// ISR TIMER
#pragma vector = TIMER1_A0_VECTOR
__interrupt
void System_Time_Tick_MiliSeconds() {
    sys_tick_ms++;

    if ((sys_tick_ms - battery_voltage_update_count) > 5000) {
        battery_voltage_update_count = sys_tick_ms;
        sprintf(current_battery_percentage, "%.0f%%", map(Get_Battery_Voltage(), MIN_VOLTAGE_BATTERY, MAX_VOLTAGE_BATTERY, 0, 100));
        __no_operation();
    }

    if ((P4IN & GPIO_USB_DETECT)) {
        if ((sys_tick_ms - blinky_battery_symbol_count) > 1000) {
            blinky_battery_symbol_count = sys_tick_ms;
            blinky_battery_symbol = !blinky_battery_symbol;
        }
    }
    else
        blinky_battery_symbol = 1;

    if (Current_Screen == CHANGING_SECREEN_MODE) {
        if ((sys_tick_ms - blinky_page_button_count) > 500) {
            blinky_page_button_count = sys_tick_ms;
            blinky_page_button = !blinky_page_button;
        }
    }
    else
        blinky_page_button = 1;

    if (Current_Screen != CHANGING_SECREEN_MODE) {
        if ((sys_tick_ms - blinky_right_cursor_count) > 500) {
            blinky_right_cursor_count = sys_tick_ms;
            blinky_right_cursor = !blinky_right_cursor;
            blinky_left_cursor_count = sys_tick_ms;
            blinky_left_cursor = !blinky_right_cursor;
        }
    }
    else {
        blinky_right_cursor = 0;
        blinky_left_cursor = 0;
    }

    if (Current_Screen == HOME_SCREEN) {
        if ((sys_tick_ms - blinky_datalogger_status_count) > 700) {
            blinky_datalogger_status_count = sys_tick_ms;
            blinky_datalogger_status = !blinky_datalogger_status;
        }
    }
    else
        blinky_datalogger_status = 1;

    if (Rotary_Encoder_Push_Button() == BUTTON_PRESSED)
        Rotary_Encoder_Switch_Holding++;
    else
        Rotary_Encoder_Switch_Holding = 0;

    if (Rotary_Encoder_Switch_Holding >= SWITCH_HOLD_TIME) {
        if (Current_Screen != CHANGING_SECREEN_MODE)
            Last_Screen = Current_Screen;
        Current_Screen = CHANGING_SECREEN_MODE;
        //Rotary_Encoder_Switch_Holding = 0;
        P4IFG &= ~GPIO_ROTARY_ENCODER_BUTTON;
    }

}
