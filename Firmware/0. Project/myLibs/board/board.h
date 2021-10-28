///////////////////////////////////////////////////////////////////////////////
// Author: Tiago Silva
// E-mail: tiagodepaulasilva@gmail.com
// 2021 - São Paulo, Brazil
///////////////////////////////////////////////////////////////////////////////

#ifndef _BOARDDEFINITIONS_H_
#define _BOARDDEFINITIONS_H_

/********************************************************
 * 				GPIOs
 *	  Micro-SD Card (Card Detect)
 *	       Rotary Encoder
 ********************************************************/
#define GPIO_ROTARY_ENCODER_SWITCH 	BIT0	// P2.0
#define GPIO_ROTARY_ENCODER_SIGNAL_A BIT1	// P2.1
#define GPIO_ROTARY_ENCODER_SIGNAL_B BIT2	// P2.2
#define GPIO_SDCARD_DETECT  			BIT3	// P2.3



/********************************************************
 * 				ADCs
 *		Battery Voltage Monitor
 ********************************************************/
#define ADC_BATTERY_VOLTAGE_PIN  		BIT0 // P1.0



/********************************************************
 * 				UART
 *		External Device (data-log)
 ********************************************************/
#define UART_TX_EXTERNAL_DEVICE_PIN  BIT1 // P1.1



/********************************************************
 * 				SPI
 *		  Micro-SD Card (Read/Write)
 *	 	   OLED Display
 *	    (SETUP on UCB0 bus)
 ********************************************************/
#define SPI_CLK_PIN  					BIT5 // P1.5
#define SPI_MISO_PIN 					BIT6 // P1.6 ('SOMI')
#define SPI_MOSI_PIN 					BIT7 // P1.7 ('SIMO')
#define SPI_SDCARD_CS_PIN 			BIT5 // P2.5
#define SPI_OLED_CS_PIN 				BIT4 // P2.4


#endif  // _BOARDDEFINITIONS_H_
