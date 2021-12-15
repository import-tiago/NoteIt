#ifndef _SCREENS_H_
#define _SCREENS_H_

#include <msp430.h>
#include <stdint.h>
#include <stdint.h>


typedef enum {
	STATUS_BAR = 0,

	CURRENT_BAUD_RATE,

	CHANGE_SCREEN_BUTTON,
} ScreenElements;

StatesFSM1 stateFSM1 = STARTING;


int HomeScreen = {
					STATUS_BAR ,
					}







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

#define MINIMUM_MEASUREMENT_TIME    0


//STATE FINITE MACHINE


uint16_t Available_TCs[] = {
							100,
							250,
							400,
							600,
							800
};

uint16_t Calibration_Points[] = {
											1,
												5,
												10,
												15,
												20,
												//25,
												30,
												//35,
												40,
												//	45,
												50,
												//60,
												//	70,
												//	80,
												//	90,
												100,
												//	110,
												//	120,
												//	130,
												//	140,
												150,
												200,
												250,
												300,
												350,
												400,
												450,
												500,
												550,
												600,
												650,
												700,
												750,
												800
};

#define NUMBER_OF_TCs					((uint16_t)(sizeof(Available_TCs) / sizeof(uint16_t)))
#define NUMBER_OF_CALIBRATION_POINTS	((uint16_t)(sizeof(Calibration_Points) / sizeof(uint16_t)))

#define NUMBER_OF_COEFFICIENTS	((uint16_t)2) // Linear and Angular, respectively
#define LINEAR_COEFFICIENT	((uint16_t)0)
#define ANGULAR_COEFFICIENT	((uint16_t)1)



#endif
