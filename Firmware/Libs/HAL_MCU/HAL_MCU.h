#ifndef HAL_MCU_H_
#define HAL_MCU_H_

/*******************************************************
*   GENERAL PURPOSE CONSTANTS
********************************************************/
#define eUSCI_A0    0
#define eUSCI_A1    1
#define eUSCI_B0    0
#define eUSCI_B1    1

void Watchdog_Init();
void Oscillator_Init();
void GPIOs_Init();
void GPIO_Interrupt_Init();
void ADC_Init();

#endif // HAL_MCU_H_
