#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <./HAL_BOARD/HAL_BOARD.h>
#include <./HAL_MCU/HAL_MCU.h>
#include <./HAL_MCU/I2C/I2C.h>
#include <./HAL_MCU/SPI/SPI.h>
#include <./RTC/DS3231.h>
#include <./DISPLAY/SSD1306.h>
#include <./SDCARD/ff.h>
#include <./SDCARD/diskio.h>
//#include <./ROTARY_ENCODER/RotaryEncoder.h>

unsigned int MSB;
unsigned int LSB;
unsigned int encoded;
unsigned int sum;
unsigned int state;

#define R_START 0x0

// Use the full-step state table (emits a code at 00 only)
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6
// Enable this to emit codes twice per step.
#define HALF_STEP

// Values returned by 'process'
// No complete step yet.
#define DIR_NONE 0x0
// Clockwise step.
#define DIR_CW 0x10
// Anti-clockwise step.
#define DIR_CCW 0x20
const unsigned char ttable[7][4] = {
// R_START
                                     {
                                     R_START,
                                       R_CW_BEGIN, R_CCW_BEGIN, R_START },
                                     // R_CW_FINAL
                                     {
                                     R_CW_NEXT,
                                       R_START, R_CW_FINAL, R_START | DIR_CW },
                                     // R_CW_BEGIN
                                     {
                                     R_CW_NEXT,
                                       R_CW_BEGIN, R_START, R_START },
                                     // R_CW_NEXT
                                     {
                                     R_CW_NEXT,
                                       R_CW_BEGIN, R_CW_FINAL, R_START },
                                     // R_CCW_BEGIN
                                     {
                                     R_CCW_NEXT,
                                       R_START, R_CCW_BEGIN, R_START },
                                     // R_CCW_FINAL
                                     {
                                     R_CCW_NEXT,
                                       R_CCW_FINAL, R_START, R_START | DIR_CCW },
                                     // R_CCW_NEXT
                                     {
                                     R_CCW_NEXT,
                                       R_CCW_FINAL, R_CCW_BEGIN, R_START }, };

int8_t *g_current_time_and_date;
float temp = 0;

int pos;

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
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
void delay_ms(uint8_t v) {
    while (v--)
        __delay_cycles(1000);
}
/*
 void updateEncoder() {
 int MSB = (P4IN & GPIO_ROTARY_ENCODER_SIGNAL_A); //MSB = most significant bit
 int LSB = (P4IN & GPIO_ROTARY_ENCODER_SIGNAL_B); //LSB = least significant bit

 int encoded = (MSB << 1) | LSB; // converting the 2-pin value into a single number
 int sum = (lastEncoded << 2) | encoded; // adding it to the previous encoded value

 if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
 encoderValue++;
 if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
 encoderValue--;

 lastEncoded = encoded; //store this value for next time
 }
 */
char array[5] = { 0 };
static uint8_t prevNextCode = 0;
static uint16_t store = 0;
// A vald CW or  CCW move returns 1, invalid returns 0.
int8_t read_rotary() {
    static int8_t rot_enc_table[] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

    prevNextCode <<= 2;
    if ((P4IN & GPIO_ROTARY_ENCODER_SIGNAL_A))
        prevNextCode |= 0x02;
    if ((P4IN & GPIO_ROTARY_ENCODER_SIGNAL_B))
        prevNextCode |= 0x01;
    prevNextCode &= 0x0f;

    // If valid then store as 16 bit data.
    if (rot_enc_table[prevNextCode]) {
        store <<= 4;
        store |= prevNextCode;
        //if (store==0xd42b) return 1;
        //if (store==0xe817) return -1;
        if ((store & 0xff) == 0x2b)
            return -1;
        if ((store & 0xff) == 0x17)
            return 1;
    }
    return 0;
}

int main(void) {

    Watchdog_Init();
    GPIOs_Init();
    // GPIO_Interrupt_Init();
    //Oscillator_Init(); //16MHz
    SPI_Master_Mode_Init(eUSCI_A0); //SDCARD
    SPI_Master_Mode_Init(eUSCI_B1); //Display OLED
    I2C_Master_Mode_Init(eUSCI_B0); //RTC

    __enable_interrupt();

    Display_Init();

    string_typer(0, 0, "TESTE", 2, 1000);

    Set_Clock_and_Calendar(58, 9, 21, 4, 23, 11, 21);
    state = R_START;
    while (1) {

        static int8_t c, val;

        if (val = read_rotary()) {
            c += val;
            //Serial.print(c);Serial.print(" ");

            if (prevNextCode == 0x0b) {

                fill_display(LCD_PIXELS_WIDTH, LCD_PIXELS_HEIGHT, 0x00);

                sprintf(array, "%d", c);
                string_typer(0, 0, array, 2, 1000);
            }

            if (prevNextCode == 0x07) {
                fill_display(LCD_PIXELS_WIDTH, LCD_PIXELS_HEIGHT, 0x00);

                sprintf(array, "%d", c);
                string_typer(0, 0, array, 2, 1000);
            }
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

         fill_display(LCD_PIXELS_WIDTH, LCD_PIXELS_HEIGHT, 0x00);

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
__interrupt void Port_4(void) {

    /*

     const static uint16_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

     static uint16_t a = 0; // the old value of the sensor ports
     static uint16_t b = 0; // the old value of the sensor ports
     static uint16_t ab = 0;
     static uint16_t angle = 0;
     */
    /*


     static unsigned char push_pos = 0;


     ab |= (port & 0x3); // OR in the two new bits
     angle += table[(ab & 0xf)]; // get the change from the 16 entry table
     */
    //  P4IE &= ~(GPIO_ROTARY_ENCODER_BUTTON + GPIO_ROTARY_ENCODER_SIGNAL_A + GPIO_ROTARY_ENCODER_SIGNAL_B);
    /*
     if ((P4IFG & GPIO_ROTARY_ENCODER_BUTTON)) {
     // P4IES ^= GPIO_ROTARY_ENCODER_BUTTON;   // Toggle the edge trigger
     P4IFG &= ~GPIO_ROTARY_ENCODER_BUTTON;  // Clear interrupt flag
     }

     if ((P4IFG & GPIO_ROTARY_ENCODER_SIGNAL_A)) {
     a = (P4IN & GPIO_ROTARY_ENCODER_SIGNAL_A);
     b = (P4IN & GPIO_ROTARY_ENCODER_SIGNAL_B) << 2;
     ab = (a | b);
     // P4IES ^= GPIO_ROTARY_ENCODER_SIGNAL_A;   // Toggle the edge trigger
     P4IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_A;  // Clear interrupt flag
     }
     */
    /*
     static uint8_t prevNextCode = 0;
     prevNextCode <<= 2;

     if ((P4IN & GPIO_ROTARY_ENCODER_SIGNAL_A))
     prevNextCode |= 0x02;

     if ((P4IN & GPIO_ROTARY_ENCODER_SIGNAL_B))
     prevNextCode |= 0x01;

     prevNextCode &= 0x0f;

     ab = rot_enc_table[( prevNextCode & 0x0f )];

     P4IFG &= ~(GPIO_ROTARY_ENCODER_SIGNAL_A + GPIO_ROTARY_ENCODER_SIGNAL_B);

     */
    /*
     if ((P4IFG & GPIO_ROTARY_ENCODER_SIGNAL_B)) {
     a = (P4IN & GPIO_ROTARY_ENCODER_SIGNAL_A);
     b = (P4IN & GPIO_ROTARY_ENCODER_SIGNAL_B) << 2;
     ab = (a | b);
     //   P4IES ^= GPIO_ROTARY_ENCODER_SIGNAL_B;   // Toggle the edge trigger
     P4IFG &= ~GPIO_ROTARY_ENCODER_SIGNAL_B;  // Clear interrupt flag
     }
     */
    /*

     char array[5] = { 0 };

     fill_display(LCD_PIXELS_WIDTH, LCD_PIXELS_HEIGHT, 0x00);

     sprintf(array, "%d", a);
     string_typer(0, 0, array, 2, 1000);

     sprintf(array, "%d", a);
     string_typer(0, 2, array, 2, 1000);

     sprintf(array, "%d", ab);
     string_typer(0, 4, array, 2, 1000);
     __no_operation();
     */
    // P4IE |= GPIO_ROTARY_ENCODER_BUTTON | GPIO_ROTARY_ENCODER_SIGNAL_A | GPIO_ROTARY_ENCODER_SIGNAL_B;
}
