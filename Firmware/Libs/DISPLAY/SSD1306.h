#ifndef SSD1306_H_
#define SSD1306_H_

#include <msp430.h>
#include <stdint.h>

#define f_width     5

// standard ascii 5x7 font
// 128 chars = 640 bytes (128*5bytes)
static const int8_t ascii_table[][f_width]={
                             0xFF,0xFF,0xFF,0xFF,0xFF,    // ASCII 00: all pixel
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 01: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 02: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 03: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 04: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 05: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 06: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 07: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 08: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 09: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 0a: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 0b: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 0c: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 0d: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 0e: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 0f: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 10: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 11: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 12: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 13: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 14: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 15: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 16: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 17: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 18: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 19: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 1a: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 1b: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 1c: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 1d: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 1e: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 1f: reserved
                             0x00,0x00,0x00,0x00,0x00,          // ASCII 20: SP (Space)
                             0x00,0x00,0x5f,0x00,0x00,          // ASCII 21: !
                             0x00,0x07,0x00,0x07,0x00,          // ASCII 22: "
                             0x14,0x7f,0x14,0x7f,0x14,          // ASCII 23: #
                             0x24,0x2a,0x7f,0x2a,0x12,          // ASCII 24: $
                             0x23,0x13,0x08,0x64,0x62,          // ASCII 25: %
                             0x36,0x49,0x55,0x22,0x50,          // ASCII 26: &
                             0x00,0x05,0x03,0x00,0x00,          // ASCII 27: '
                             0x00,0x1c,0x22,0x41,0x00,          // ASCII 28: (
                             0x00,0x41,0x22,0x1c,0x00,          // ASCII 29: )
                             0x14,0x08,0x3e,0x08,0x14,          // ASCII 2a: *
                             0x08,0x08,0x3e,0x08,0x08,          // ASCII 2b: +
                             0x00,0x50,0x30,0x00,0x00,          // ASCII 2c: ,
                             0x08,0x08,0x08,0x08,0x08,          // ASCII 2d: -
                             0x00,0x60,0x60,0x00,0x00,          // ASCII 2e: .
                             0x20,0x10,0x08,0x04,0x02,          // ASCII 2f: /
                             0x3e,0x51,0x49,0x45,0x3e,          // ASCII 30: 0
                             0x00,0x42,0x7f,0x40,0x00,          // ASCII 31: 1
                             0x42,0x61,0x51,0x49,0x46,          // ASCII 32: 2
                             0x21,0x41,0x45,0x4b,0x31,          // ASCII 33: 3
                             0x18,0x14,0x12,0x7f,0x10,          // ASCII 34: 4
                             0x27,0x45,0x45,0x45,0x39,          // ASCII 35: 5
                             0x3c,0x4a,0x49,0x49,0x30,          // ASCII 36: 6
                             0x01,0x71,0x09,0x05,0x03,          // ASCII 37: 7
                             0x36,0x49,0x49,0x49,0x36,          // ASCII 38: 8
                             0x06,0x49,0x49,0x29,0x1e,          // ASCII 39: 9
                             0x00,0x36,0x36,0x00,0x00,          // ASCII 3a: :
                             0x00,0x56,0x36,0x00,0x00,          // ASCII 3b: ;
                             0x08,0x14,0x22,0x41,0x00,          // ASCII 3c: <
                             0x14,0x14,0x14,0x14,0x14,          // ASCII 3d: =
                             0x00,0x41,0x22,0x14,0x08,          // ASCII 3e: >
                             0x02,0x01,0x51,0x09,0x06,          // ASCII 3f: ?
                             0x32,0x49,0x79,0x41,0x3e,          // ASCII 40: @
                             0x7e,0x11,0x11,0x11,0x7e,          // ASCII 41: A
                             0x7f,0x49,0x49,0x49,0x36,          // ASCII 42: B
                             0x3e,0x41,0x41,0x41,0x22,          // ASCII 43: C
                             0x7f,0x41,0x41,0x22,0x1c,          // ASCII 44: D
                             0x7f,0x49,0x49,0x49,0x41,          // ASCII 45: E
                             0x7f,0x09,0x09,0x09,0x01,          // ASCII 46: F
                             0x3e,0x41,0x49,0x49,0x7a,          // ASCII 47: G
                             0x7f,0x08,0x08,0x08,0x7f,          // ASCII 48: H
                             0x00,0x41,0x7f,0x41,0x00,          // ASCII 49: I
                             0x20,0x40,0x41,0x3f,0x01,          // ASCII 4a: J
                             0x7f,0x08,0x14,0x22,0x41,          // ASCII 4b: K
                             0x7f,0x40,0x40,0x40,0x40,          // ASCII 4c: L
                             0x7f,0x02,0x0c,0x02,0x7f,          // ASCII 4d: M
                             0x7f,0x04,0x08,0x10,0x7f,          // ASCII 4e: N
                             0x3e,0x41,0x41,0x41,0x3e,          // ASCII 4f: O
                             0x7f,0x09,0x09,0x09,0x06,          // ASCII 50: P
                             0x3e,0x41,0x51,0x21,0x5e,          // ASCII 51: Q
                             0x7f,0x09,0x19,0x29,0x46,          // ASCII 52: R
                             0x46,0x49,0x49,0x49,0x31,          // ASCII 53: S
                             0x01,0x01,0x7f,0x01,0x01,          // ASCII 54: T
                             0x3f,0x40,0x40,0x40,0x3f,          // ASCII 55: U
                             0x1f,0x20,0x40,0x20,0x1f,          // ASCII 56: V
                             0x3f,0x40,0x38,0x40,0x3f,          // ASCII 57: W
                             0x63,0x14,0x08,0x14,0x63,          // ASCII 58: X
                             0x07,0x08,0x70,0x08,0x07,          // ASCII 59: Y
                             0x61,0x51,0x49,0x45,0x43,          // ASCII 5a: Z
                             0x00,0x7f,0x41,0x41,0x00,          // ASCII 5b: [
                             0x02,0x04,0x08,0x10,0x20,          // ASCII 5c: ¥
                             0x00,0x41,0x41,0x7f,0x00,          // ASCII 5d: ]
                             0x04,0x02,0x01,0x02,0x04,          // ASCII 5e: ^
                             0x40,0x40,0x40,0x40,0x40,          // ASCII 5f: _
                             0x00,0x01,0x02,0x04,0x00,          // ASCII 60: `
                             0x20,0x54,0x54,0x54,0x78,          // ASCII 61: a
                             0x7f,0x48,0x44,0x44,0x38,          // ASCII 62: b
                             0x38,0x44,0x44,0x44,0x20,          // ASCII 63: c
                             0x38,0x44,0x44,0x48,0x7f,          // ASCII 64: d
                             0x38,0x54,0x54,0x54,0x18,          // ASCII 65: e
                             0x08,0x7e,0x09,0x01,0x02,          // ASCII 66: f
                             0x0c,0x52,0x52,0x52,0x3e,          // ASCII 67: g
                             0x7f,0x08,0x04,0x04,0x78,          // ASCII 68: h
                             0x00,0x44,0x7d,0x40,0x00,          // ASCII 69: i
                             0x20,0x40,0x44,0x3d,0x00,          // ASCII 6a: j
                             0x7f,0x10,0x28,0x44,0x00,          // ASCII 6b: k
                             0x00,0x41,0x7f,0x40,0x00,          // ASCII 6c: l
                             0x7c,0x04,0x18,0x04,0x78,          // ASCII 6d: m
                             0x7c,0x08,0x04,0x04,0x78,          // ASCII 6e: n
                             0x38,0x44,0x44,0x44,0x38,          // ASCII 6f: o
                             0x7c,0x14,0x14,0x14,0x08,          // ASCII 70: p
                             0x08,0x14,0x14,0x18,0x7c,          // ASCII 71: q
                             0x7c,0x08,0x04,0x04,0x08,          // ASCII 72: r
                             0x48,0x54,0x54,0x54,0x20,          // ASCII 73: s
                             0x04,0x3f,0x44,0x40,0x20,          // ASCII 74: t
                             0x3c,0x40,0x40,0x20,0x7c,          // ASCII 75: u
                             0x1c,0x20,0x40,0x20,0x1c,          // ASCII 76: v
                             0x3c,0x40,0x30,0x40,0x3c,          // ASCII 77: w
                             0x44,0x28,0x10,0x28,0x44,          // ASCII 78: x
                             0x0c,0x50,0x50,0x50,0x3c,          // ASCII 79: y
                             0x44,0x64,0x54,0x4c,0x44,          // ASCII 7a: z
                             0x00,0x08,0x36,0x41,0x00,          // ASCII 7b: {
                             0x00,0x00,0x7f,0x00,0x00,          // ASCII 7c: |
                             0x00,0x41,0x36,0x08,0x00,          // ASCII 7d: }
                             0x10,0x08,0x08,0x10,0x08,          // ASCII 7e: ->
                             0x78,0x46,0x41,0x46,0x78,          // ASCII 7f: <-
                             0x00,0x06,0x09,0x09,0x06          // degree symbol //0x04,  0x0A,  0x11,  0x0A,  0x04 //
                             };

static const uint8_t display_initialization_sequence[] = { 0xAE,           // DISPLAY OFF
                                          0xD5,           // SET OSC FREQUENY
                                          0x80,           // divide ratio = 1 (bit 3-0), OSC (bit 7-4)
                                          0xA8,           // SET MUX RATIO
                                          0x3F,           // 64MUX
                                          0xD3,           // SET DISPLAY OFFSET
                                          0x00,           // offset = 0
                                          0x40,           // set display start line, start line = 0
                                          0x8D,           // ENABLE CHARGE PUMP REGULATOR
                                          0x14,           //
                                          0x20,           // SET MEMORY ADDRESSING MODE
                                          0x02,           // horizontal addressing mode
                                          0xA1,           // set segment re-map, column address 127 is mapped to SEG0
                                          0xC8,           // set COM/Output scan direction, remapped mode (COM[N-1] to COM0)
                                          0xDA,           // SET COM PINS HARDWARE CONFIGURATION
                                          0x12,           // alternative COM pin configuration
                                          0x81,           // SET CONTRAST CONTROL
                                          0xCF,           //
                                          0xD9,           // SET PRE CHARGE PERIOD
                                          0xF1,           //
                                          0xDB,           // SET V_COMH DESELECT LEVEL
                                          0x40,           //
                                          0xA4,           // DISABLE ENTIRE DISPLAY ON
                                          0xA6,           // NORMAL MODE (A7 for inverse display)
                                          0xAF };         // DISPLAY ON

#define DISPLAY_PIXELS_WIDTH		128
#define DISPLAY_PIXELS_HEIGHT     64


#define COMMAND_MODE 0
#define DATA_MODE    1

#define space_char	1		// space between chars

void Display_Init(void);
void set_instruction(unsigned char transmission_type, unsigned char number);
void fill_display(unsigned char width, unsigned char height, unsigned char byte);
void set_cursor(unsigned char x, unsigned char y);
void string_typer(unsigned char x, unsigned char y, const char *text, unsigned char f_size, unsigned int ms);
void write_char(unsigned char x, unsigned char y, unsigned char character, unsigned char f_size);
void wait_ms(unsigned int m_sec);
void send_data_array(const char *d_array, unsigned char size);
void convert_font_size(unsigned char x, unsigned char y, unsigned char character, unsigned char f_size);
void OLED_Display_Clear();
void drawImage(unsigned char x, unsigned char y, unsigned char sx, unsigned char sy, const unsigned char img[], unsigned char invert) ;
#endif // SSD1306_H_
