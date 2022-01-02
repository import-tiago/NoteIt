#ifndef _SCREENS_H_
#define _SCREENS_H_

#include <msp430.h>
#include <stdint.h>
#include <stdint.h>

// 'CHECKED_BUTTON', 10x10px
const unsigned char Bitmap_CHECKED_BUTTON[] = { 0x00, 0x00, 0x3f, 0x00, 0x5e, 0x80, 0x6d, 0x80, 0x73, 0x80, 0x73, 0x80, 0x6d, 0x80, 0x5e, 0x80, 0x3f, 0x00, 0x00, 0x00 };
// 'CHECK_BUTTON', 10x10px
const unsigned char Bitmap_CHECK_BUTTON[] = { 0x00, 0x00, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x7f, 0x80, 0x00, 0x00 };
// 'CUREENT_PAGE2', 5x5px
const unsigned char current_page_bitmap[] = { 0xf8, 0x88, 0x88, 0x88, 0xf8 };


uint32_t Baudrate_List[] = {
                            1200,
                            2400,
                            4800,
                            9600,
                            14400,
                            19200,
                            28800,
                            38400,
                            57600,
                            115200,
                            230400
};
#define BAUDRATE_LIST_LENGTH ((uint32_t)(sizeof(Baudrate_List) / sizeof(uint32_t)))

#define SCREEN_ID 0
#define SINGLE_POSITION 0

enum ScreensList {
    HOME_SCREEN,
    LOG_SETTINGS_SCREEN,
    CLOCK_AND_DATE_SCREEN,
    NUMBER_OF_SCREENS
};

enum ScreenElementsList {
    STATUS_BAR,
    CURRENT_BAUD_RATE,
    DATALOGGER_STATE,
    SCREENS_NAVIGATION_BUTTONS,
    BAUD_RATE_SELECTION,
    LOG_INSERT_TEMPERATURE,
    LOG_INSERT_TIME,
    LOG_INSERT_DATE,
    LOG_INSERT_EPOCH_TIMESTAMP,
    CLOCK_ADJUSTMENT,
    CALENDAR_ADJUSTMENT,
    NUMBER_SCREEN_ELEMENTS
};

enum Adjustments_Possibilities_in_Elements {
    NO_ADJUSTMENTS_AVAILABLE,
    TWO_OPTIONS,
    THREE_OPTIONS,
    FOUR_OPTIONS,
    FIVE_OPTIONS
};

 const struct ScreensStruct {
    uint8_t Home_Screen_Parameters[4][3][1];
    uint8_t Log_Settings_Screen_Parameters[6][3][1];
    uint8_t Clock_and_Calendar_Screen_Parameters[4][3][1];

} Screens = {
   .Home_Screen_Parameters = {
       {{HOME_SCREEN},           {STATUS_BAR},                 {NO_ADJUSTMENTS_AVAILABLE}},
       {{HOME_SCREEN},           {CURRENT_BAUD_RATE},          {BAUDRATE_LIST_LENGTH}},
       {{HOME_SCREEN},           {DATALOGGER_STATE},           {BAUDRATE_LIST_LENGTH}},
       {{HOME_SCREEN},           {SCREENS_NAVIGATION_BUTTONS}, {NUMBER_OF_SCREENS}}
    },
   .Log_Settings_Screen_Parameters = {
       {{LOG_SETTINGS_SCREEN},   {STATUS_BAR},                 {NO_ADJUSTMENTS_AVAILABLE}},
       {{LOG_SETTINGS_SCREEN},   {LOG_INSERT_TIME},            {TWO_OPTIONS}},
       {{LOG_SETTINGS_SCREEN},   {LOG_INSERT_DATE},            {TWO_OPTIONS}},
       {{LOG_SETTINGS_SCREEN},   {LOG_INSERT_EPOCH_TIMESTAMP}, {TWO_OPTIONS}},
       {{LOG_SETTINGS_SCREEN},   {LOG_INSERT_TEMPERATURE},     {TWO_OPTIONS}},
       {{LOG_SETTINGS_SCREEN},   {SCREENS_NAVIGATION_BUTTONS}, {NUMBER_OF_SCREENS}}
   },
   .Clock_and_Calendar_Screen_Parameters = {
       {{CLOCK_AND_DATE_SCREEN}, {STATUS_BAR},                 {NO_ADJUSTMENTS_AVAILABLE}},
       {{CLOCK_AND_DATE_SCREEN}, {CLOCK_ADJUSTMENT},           {TWO_OPTIONS}},
       {{CLOCK_AND_DATE_SCREEN}, {CALENDAR_ADJUSTMENT},        {THREE_OPTIONS}},
       {{CLOCK_AND_DATE_SCREEN}, {SCREENS_NAVIGATION_BUTTONS}, {NUMBER_OF_SCREENS}}
   }
};

#define HOME_SCREEN_NUMBER_OF_ELEMENTS ((sizeof(Screens.Home_Screen_Parameters) / sizeof(Screens.Home_Screen_Parameters[0]))-1)
#define LOG_SETTINGS_SCREEN_NUMBER_OF_ELEMENTS ((sizeof(Screens.Log_Settings_Screen_Parameters) / sizeof(Screens.Log_Settings_Screen_Parameters[0]))-1)
#define CLOCK_AND_CALENDAR_SCREEN_NUMBER_OF_ELEMENTS ((sizeof(Screens.Clock_and_Calendar_Screen_Parameters) / sizeof(Screens.Clock_and_Calendar_Screen_Parameters[0]))-1)











#define BAUDRATE_SCREEN_NUMBER_OF_ELEMENTS ((uint8_t)(sizeof(Baudrate_Screen) / sizeof(uint8_t)))


#define LOG_SETTINGS_NUMBER_OF_ELEMENTS ((uint8_t)(sizeof(Log_Settings_Screen) / sizeof(uint8_t)))


#define CLOCK_AND_CALENDAR_NUMBER_OF_ELEMENTS  ((uint8_t)(sizeof(Clock_and_Calendar_Screen) / sizeof(uint8_t)))






#endif
