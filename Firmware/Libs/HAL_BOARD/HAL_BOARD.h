#ifndef HAL_BOARD_H_
#define HAL_BOARD_H_

#include <msp430.h>

/*******************************************************
*	GPIOs - INPUTS and OUTPUTS
********************************************************/
#define GPIO_ROTARY_ENCODER_BUTTON   	BIT0    //P4.0
#define GPIO_ROTARY_ENCODER_SIGNAL_A 	BIT1    //P4.1
#define GPIO_ROTARY_ENCODER_SIGNAL_B	BIT2    //P4.2

#define GPIO_OLED_TRANSMISSION_MODE	BIT7    //P3.7
#define GPIO_OLED_RESET 	BIT2    //P6.2

#define GPIO_USB_DETECT 				BIT5    //P4.5
#define GPIO_SDCARD_DETECT 			BIT6    //P4.6
#define GPIO_EXTERNAL_SUPPLY_DETECT 	BIT7    //P4.7


/*******************************************************
*	ADC - BATTERY
********************************************************/
#define ADC_BATTERY_VOLTAGE    		BIT0    //P1.0

/*******************************************************
*	UART - eUSCI A1 - DATA LOGGER
********************************************************/
#define UART_UCA1_RX    				BIT5    //P2.5

/*******************************************************
*	SPI - eUSCI A0 - SD CARD
********************************************************/
#define SPI_UCA0_SIMO   				BIT4    //P1.4
#define SPI_UCA0_SOMI 					BIT5    //P1.5
#define SPI_UCA0_CLK  					BIT6    //P1.6
#define SPI_SDCARD_CS   				BIT7    //P1.7

/*******************************************************
*	SPI - eUSCI B1 - DISPLAY OLED
********************************************************/
#define SPI_UCB1_SIMO   				BIT4    //P4.4
#define SPI_UCB1_SOMI 					BIT3    //P4.3
#define SPI_UCB1_CLK  					BIT3    //P5.3
#define SPI_OLED_CS     				BIT4    //P5.4

/*******************************************************
*	I2C - eUSCI B0 - Real Time Clock (DS3231)
********************************************************/
#define I2C_UCB0_SDA    				BIT2    //P1.2
#define I2C_UCB0_SCL 					BIT3    //P1.3

/*******************************************************
*	EXTERNAL OSCILLATOR (32 kHz)
********************************************************/
#define XTAL_IN    					BIT0    //P2.0
#define XTAL_OUT 						BIT1    //P2.1



#endif // HAL_BOARD_H_
