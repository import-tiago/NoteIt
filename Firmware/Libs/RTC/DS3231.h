// dsLib.h
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Inspired by https://github.com/mizraith/RTClib/blob/master/RTC_DS3231.cpp
//
// Requirements:
//	dsLib.c
//
// Description:
// 	Interface with DS3231 to get time
//
// Notes:
//
// Todo:
//	Make more durable, timeouts, testing, etc.
//****************************************************************************************************

#ifndef DSLIB_H_
#define DSLIB_H_



// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>
#include <msp430.h>



// Defines -------------------------------------------------------------------------------------------
#define DS3231_I2C_ADDRESS    0x68
#define DS3231_REG_SECONDS    0x00
#define DS3231_REG_MINUTES    0x01
#define DS3231_REG_HOURS      0x02
#define DS3231_REG_DAYOFWEEK  0x03
#define DS3231_REG_DAYOFMONTH 0x04
#define DS3231_REG_MONTH      0x05
#define DS3231_REG_YEAR       0x06
#define DS3231_REG_A1SECONDS  0x07
#define DS3231_REG_A1MINUTES  0x08
#define DS3231_REG_A1HOURS    0x09
#define DS3231_REG_A1DAYDATE  0x0A
#define DS3231_REG_A2MINUTES  0x0B
#define DS3231_REG_A2HOURS    0x0C
#define DS3231_REG_A2DAYDATE  0x0D
#define DS3231_REG_CONTROL    0x0E
#define DS3231_REG_STATUS_CTL 0x0F
#define DS3231_REG_AGING      0x10
#define DS3231_REG_TEMP_MSB   0x11
#define DS3231_REG_TEMP_LSB   0x12
#define DS3231_TIME_LENGTH	  0x07



// Global --------------------------------------------------------------------------------------------
uint8_t date_and_time_array[7];	//{seconds, minutes, hours, day, date, month, year}

void I2C_Init(uint8_t slave_addr);

uint8_t BCD_to_DEC(uint8_t value);
uint8_t DEC_to_BCD(uint8_t value);

void Get_Current_Time_and_Date(void);
void Set_Current_Time_and_Date(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t dayOfMonth, uint8_t month, uint8_t year);



extern uint8_t roundUpToTen(uint8_t val);
extern void DS3231SetAlarm1Plus10Sec(void);
extern void DS3231SetAlarm1Round10Sec(void);
extern void DS3231TurnAlarm1On(void);
extern void DS3231TurnAlarm1Off(void);
extern void DS3231ClearAlarm1Bits(void);
unsigned int read_register(unsigned int reg_addr);

#endif /* DSLIB_H_ */
