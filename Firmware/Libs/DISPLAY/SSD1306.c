#include <stdint.h>
#include <stdio.h>

#include <./DISPLAY/SSD1306.h>
#include <./HAL_MCU/SPI/SPI.h>
#include <./HAL_BOARD/HAL_BOARD.h>

unsigned char byte, bit_num;        // global buffer and bit index
#define max_f_size      8       // max. font cache size for resize function
unsigned char *dataBuffer;


void Display_Init() {
    uint8_t total_commands = sizeof(display_initialization_sequence);
    uint8_t current_command = total_commands;

    P6OUT |= GPIO_OLED_RESET;

    while (current_command) {
        set_instruction(COMMAND_MODE, display_initialization_sequence[total_commands - current_command]);
        current_command--;
    }

    fill_display(DISPLAY_PIXELS_WIDTH, DISPLAY_PIXELS_HEIGHT, 0x00); // display RAM is undefined after reset, clean dat shit
}

void set_instruction(unsigned char transmission_mode, unsigned char number) {

    if (transmission_mode == DATA_MODE)
        P3OUT |= GPIO_OLED_TRANSMISSION_MODE; //  Transmit a data

    else if (transmission_mode == COMMAND_MODE)
        P3OUT &= ~GPIO_OLED_TRANSMISSION_MODE; // Transmit a command

    P5OUT &= ~SPI_OLED_CS;			// Select SPI target slave
    while (!(UCB1IFG & UCTXIFG));		// Checks if TX buffer is ready
    UCB1TXBUF = number;			    // Start transmission
}

void fill_display(unsigned char width, unsigned char height, unsigned char byte) {
    height /= 8;
    while (height--) {
        set_cursor(0, height);
        while (width--)
            set_instruction(DATA_MODE, byte);
        width = DISPLAY_PIXELS_WIDTH;
    }
}

void set_cursor(unsigned char x, unsigned char y) {
    set_instruction(COMMAND_MODE, 0x0F & x);			    // set lower nibble of the column start address
    set_instruction(COMMAND_MODE, 0x10 + (x >> 4));			    // set higher nibble of the column start address
    set_instruction(COMMAND_MODE, 0xB0 + y);
}

void wait_ms(unsigned int m_sec) {
    //  while (m_sec--)
    //       __delay_cycles(1000);
}

void string_typer(unsigned char x, unsigned char y, const char *text, unsigned char f_size, unsigned int ms) {

    while (*text != 0) {
        write_char(x, y, *text++, f_size);
        if (f_size > 1)
            x += f_size;
        else
            x++;
        wait_ms(ms);
    }
}

void OLED_Display_Clear() {
    fill_display(DISPLAY_PIXELS_WIDTH, DISPLAY_PIXELS_HEIGHT, 0x00);
}

void write_char(unsigned char x, unsigned char y, unsigned char character, unsigned char f_size) {
    x *= (f_width + space_char);
    set_cursor(x, y);
    if (f_size)
        convert_font_size(x, y, character, f_size);
    else
        send_data_array(ascii_table[character], f_width);
}

void send_data_array(const char *d_array, unsigned char size) {
    while (size--)
        set_instruction(DATA_MODE, *d_array++);
}

// horizontal resize with cache (every bit in one byte will be resized to f_size, for example:
// f_size=2, every pixel will have a new size of 2*2 pixel, f_size=3, pixel-size: 3*3 pixel etc.
// f_size=1, double height font, only horizontal duplication of pixels (bits), pixel-size: 1*2 pixel
// the vertical duplication is running over a cache (every horizontal row (resized byte to f_size),
// will be write "f_size"-times to the display)
void convert_font_size(unsigned char x, unsigned char y, unsigned char character, unsigned char f_size) {
    unsigned char x_char = 0, bit_num_b = 0, size = 0, px_size = 0, y_pos_new = 0, x_pos_new = 0;
    unsigned char cache[max_f_size], i = 0;
    byte = 0;
    bit_num = 0;

    if (f_size == 1)
        size = 2;
    else
        size = f_size;

    while (x_char < f_width)    // test byte, starting at 0 to f_width (font width)
    {
        while (bit_num < 8)                     // test bit 0..7 of current byte
        {
            if (ascii_table[character][x_char] & (1 << bit_num))                        // if bit=1 in byte...
                    {
                while (px_size < size)      // duplicate bits (f_size*f_size)
                {
                    if (bit_num_b > 7 && px_size > 0)       // byte overflow, new byte
                            {
                        set_cursor(x + x_pos_new, y + y_pos_new++); // set cursor (increment y-new position)
                        set_instruction(DATA_MODE, byte);                   // send byte
                        bit_num_b = 0;          // reset bit counter (buffer)
                        cache[i++] = byte;              // save byte in cache
                        byte = 0;                               // reset byte
                    }
                    byte |= (1 << bit_num_b);               // set bit in byte
                    px_size++;              // increment pixel duplicate counter
                    bit_num_b++;            // increment bit position for buffer
                }
                px_size = 0;                    // reset pixel duplicate counter
            }
            else
                bit_num_b += size;  // bit=0, calculate new bit position in byte
                                    // if bit=0, remaining bits are 0, too

            if (bit_num_b > 7)                      // byte overflow, new byte
                    {
                set_cursor(x + x_pos_new, y + y_pos_new++);
                set_instruction(DATA_MODE, byte);
                bit_num_b -= 8;
                cache[i++] = byte;
                byte = 0;
            }
            bit_num++;                              // test next byte in array
        }
        y_pos_new = 0;                                      // reset y-offset
        x_pos_new++;                // increment x-position
        i = 0;                                          // reset cache counter
        if (f_size == 1)
            size = 0;               // double height font (only for f_size=1)
        else
            size--;                         // first row is ready, only size-1
        while (size--) {
            while (i < f_size) {
                set_cursor(x + x_pos_new, y + y_pos_new++);
                set_instruction(DATA_MODE, cache[i++]);     // horizontal cache write
            }
            i = 0;
            y_pos_new = 0;
            x_pos_new++;
        }
        x_char++;                    // increment byte position
        if (f_size == 1)
            size = 2;        // size correction
        else
            size = f_size;
        i = 0;                      // reset cache counter
        bit_num = 0;
    }
}

void drawImage(unsigned char x, unsigned char y, unsigned char sx, unsigned char sy, const unsigned char img[], unsigned char invert) {
    unsigned int j, t;
    unsigned char i, p, p0, p1, n, n1, b;

    if (((x + sx) > DISPLAY_PIXELS_WIDTH) || ((y + sy) > DISPLAY_PIXELS_HEIGHT) || (sx == 0) || (sy == 0))
        return;

    // Total bytes of the image array
    if (sy % 8)
        t = (sy / 8 + 1) * sx;
    else
        t = (sy / 8) * sx;
    p0 = y / 8;                 // first page index
    p1 = (y + sy - 1) / 8;      // last page index
    n = y % 8;                  // offset form begin of page

    n1 = (y + sy) % 8;
    if (n1)
        n1 = 8 - n1;

    j = 0;                      // bytes counter [0..t], or [0..(t+sx)]
    dataBuffer = malloc(sx + 1);       // allocate memory for the buf
    //dataBuffer[0] = SSD1306_DATA_MODE; // fist item "send data mode"
    for (p = p0; p < (p1 + 1); p++) {
        //setCursor(x, p);
        for (i = x; i < (x + sx); i++) {
            if (p == p0) {
                b = (img[j] << n) & 0xFF;
            }
            else if ((p == p1) && (j >= t)) {
                b = (img[j - sx] >> n1) & 0xFF;
            }
            else {
                b = ((img[j - sx] >> (8 - n)) & 0xFF) | ((img[j] << n) & 0xFF);
            };
            if (invert)
                dataBuffer[i - x + 1] = b;
            else
                dataBuffer[i - x + 1] = ~b;
            j++;
        }
        set_cursor(x, p);
        set_instruction(DATA_MODE, dataBuffer); // send the buf to display
    }
    free(dataBuffer);
}
