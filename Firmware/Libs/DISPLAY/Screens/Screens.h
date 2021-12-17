#ifndef _SCREENS_H_
#define _SCREENS_H_

#include <msp430.h>
#include <stdint.h>
#include <stdint.h>

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
    CHANGE_SCREEN_BUTTON,
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


/*
uint8_t HomeScreen[][2][1] = {
                                 {{STATUS_BAR}, {0}},
                                 {{CURRENT_BAUD_RATE}, {0}},
                                 {{CHANGE_SCREEN_BUTTON}, {NUMBER_OF_SCREENS}}
                              };
*/
#define HOME_SCREEN_NUMBER_OF_ELEMENTS ((uint8_t)(sizeof(HomeScreen) / sizeof(uint8_t)))


#define BAUDRATE_SCREEN_NUMBER_OF_ELEMENTS ((uint8_t)(sizeof(Baudrate_Screen) / sizeof(uint8_t)))


#define LOG_SETTINGS_NUMBER_OF_ELEMENTS ((uint8_t)(sizeof(Log_Settings_Screen) / sizeof(uint8_t)))


#define CLOCK_AND_CALENDAR_NUMBER_OF_ELEMENTS  ((uint8_t)(sizeof(Clock_and_Calendar_Screen) / sizeof(uint8_t)))






#endif
