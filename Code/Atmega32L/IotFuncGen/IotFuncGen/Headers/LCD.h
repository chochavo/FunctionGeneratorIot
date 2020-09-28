#include "main.h"
#ifndef LCD_H_
#define LCD_H_

#define lcd_D7_port     PORTD                   // lcd D7 connection
#define lcd_D7_bit      7
#define lcd_D7_ddr      DDRD

#define lcd_D6_port     PORTD                   // lcd D6 connection
#define lcd_D6_bit      6
#define lcd_D6_ddr      DDRD

#define lcd_D5_port     PORTD                   // lcd D5 connection
#define lcd_D5_bit      5
#define lcd_D5_ddr      DDRD

#define lcd_D4_port     PORTD                   // lcd D4 connection
#define lcd_D4_bit      4
#define lcd_D4_ddr      DDRD

#define lcd_E_port      PORTB                   // lcd Enable pin
#define lcd_E_bit       1
#define lcd_E_ddr       DDRB

#define lcd_RS_port     PORTB                   // lcd Register Select pin
#define lcd_RS_bit      0
#define lcd_RS_ddr      DDRB

// LCD module information
#define LCD_LINE_1     0x00                    // start of line 1
#define LCD_LINE_2     0x40                    // start of line 2
#define LCD_LINE_3	   0x14                  // start of line 3 (20x4)
#define LCD_LINE_4     0x54                  // start of line 4 (20x4)
// LCD instructions
#define lcd_Clear           0b00000001          // replace all characters with ASCII 'space'
#define lcd_Home            0b00000010          // return cursor to first position on first line
#define lcd_EntryMode       0b00000110          // shift cursor from left to right on read/write
#define lcd_DisplayOff      0b00001000          // turn display off
#define lcd_DisplayOn       0b00001100          // display on, cursor off, don't blink character
#define lcd_FunctionReset   0b00110000          // reset the LCD
#define lcd_FunctionSet4bit 0b00101000          // 4-bit data, 2-line display, 5 x 7 font
#define LCD_SET_CURSOR       0b10000000          // set cursor position

void LCD_Init();
void LCD_Write4Bit(uint8_t);
void LCD_WriteInstruction4Bit(uint8_t);
void LCD_WriteCharacter4Bit(uint8_t);
void LCD_WriteString4Bit(char *);
void LCD_Init4bit(void);
void LCD_LogoDisplay();
void LCD_PrintLine(char *input_string, uint8_t line_number);
void LCD_ClearLine(uint8_t line);
void LCD_Clear();
void LCD_PrintChar(uint8_t ch_in, uint8_t line, uint8_t position);
#endif