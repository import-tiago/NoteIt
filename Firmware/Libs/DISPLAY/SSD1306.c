#include <stdint.h>
#include <stdio.h>

#include <./DISPLAY/SSD1306.h>
#include <./HAL_MCU/SPI/SPI.h>
#include <./HAL_BOARD/HAL_BOARD.h>

unsigned char byte, bit_num;        // global buffer and bit index
#define max_f_size      8       // max. font cache size for resize function

void Display_Init() {
    uint8_t total_commands = sizeof(display_initialization_sequence);
    uint8_t current_command = total_commands;

    P6OUT |= GPIO_OLED_RESET;

    while (current_command) {
        set_instruction(COMMAND_MODE, display_initialization_sequence[total_commands - current_command]);
        current_command--;
    }

    fill_display(LCD_PIXELS_WIDTH, LCD_PIXELS_HEIGHT, 0x00); // display RAM is undefined after reset, clean dat shit
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
            set_instruction(1, byte);
        width = LCD_PIXELS_WIDTH;
    }
}

void set_cursor(unsigned char x, unsigned char y) {
    set_instruction(0, 0x0F & x);			    // set lower nibble of the column start address
    set_instruction(0, 0x10 + (x >> 4));			    // set higher nibble of the column start address
    set_instruction(0, 0xB0 + y);
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
        set_instruction(1, *d_array++);
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
                        set_instruction(1, byte);                   // send byte
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
                set_instruction(1, byte);
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
                set_instruction(1, cache[i++]);     // horizontal cache write
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
