#include "Headers/main.h"
#include "Headers/SPIMaster.h"
#include "Headers/LCD.h"

void LCD_Init(void)
{
	lcd_D7_ddr |= (1<<lcd_D7_bit);                  // 4 data lines - output
	lcd_D6_ddr |= (1<<lcd_D6_bit);
	lcd_D5_ddr |= (1<<lcd_D5_bit);
	lcd_D4_ddr |= (1<<lcd_D4_bit);
	lcd_E_ddr  |= (1<<lcd_E_bit);                    // E line - output
	lcd_RS_ddr |= (1<<lcd_RS_bit);                  // RS line - output
}

void LCD_Init4bit(void)
{
	// Power-up delay
	_delay_ms(100);                                 // initial 40 mSec delay

	// Set up the RS and E lines for the 'LCD_Write4Bit' subroutine.
	lcd_RS_port &= ~(1<<lcd_RS_bit);                // select the Instruction Register (RS low)
	lcd_E_port &= ~(1<<lcd_E_bit);                  // make sure E is initially low

	// Reset the LCD controller
	LCD_Write4Bit(lcd_FunctionReset);                 // first part of reset sequence
	_delay_ms(20);                                  // 4.1 mS delay (min)

	LCD_Write4Bit(lcd_FunctionReset);                 // second part of reset sequence
	_delay_us(300);                                 // 100uS delay (min)

	LCD_Write4Bit(lcd_FunctionReset);                 // third part of reset sequence
	_delay_us(300);                                 // this delay is omitted in the data sheet
	
	LCD_Write4Bit(lcd_FunctionSet4bit);               // set 4-bit mode
	_delay_us(90);                                  // 40uS delay (min)

	// Function Set instruction
	LCD_WriteInstruction4Bit(lcd_FunctionSet4bit);   // set mode, lines, and font
	_delay_us(90);                                  // 40uS delay (min)

	// Display On/Off Control instruction
	LCD_WriteInstruction4Bit(lcd_DisplayOff);        // turn display OFF
	_delay_us(90);                                  // 40uS delay (min)

	// Clear Display instruction
	LCD_WriteInstruction4Bit(lcd_Clear);             // clear display RAM
	_delay_ms(8);                                   // 1.64 mS delay (min)

	// ; Entry Mode Set instruction
	LCD_WriteInstruction4Bit(lcd_EntryMode);         // set desired shift characteristics
	_delay_us(90);                                  // 40uS delay (min)

	// Display On/Off Control instruction
	LCD_WriteInstruction4Bit(lcd_DisplayOn);         // turn the display ON
	_delay_us(90);                                  // 40uS delay (min)
}


void LCD_WriteString4Bit(char theString[]) {
	volatile int i = 0;                             // character counter*/
	while (theString[i] != 0)
	{
	LCD_WriteCharacter4Bit(theString[i]);
	i++;
	_delay_us(160);                              // 40 uS delay (min)
	}
	}

void LCD_WriteCharacter4Bit(uint8_t theData) {
	lcd_RS_port |= (1<<lcd_RS_bit);                 // select the Data Register (RS high)
	lcd_E_port &= ~(1<<lcd_E_bit);                  // make sure E is initially low
	LCD_Write4Bit(theData);                           // write the upper 4-bits of the data
	LCD_Write4Bit(theData << 4);                      // write the lower 4-bits of the data
}

void LCD_WriteInstruction4Bit(uint8_t theInstruction) {
	lcd_RS_port &= ~(1<<lcd_RS_bit);                // select the Instruction Register (RS low)
	lcd_E_port &= ~(1<<lcd_E_bit);                  // make sure E is initially low
	LCD_Write4Bit(theInstruction);                    // write the upper 4-bits of the data
	LCD_Write4Bit(theInstruction << 4);               // write the lower 4-bits of the data
}

void LCD_Write4Bit(uint8_t theByte) {
	lcd_D7_port &= ~(1<<lcd_D7_bit);                        // assume that data is '0'
	if (theByte & 1<<7) lcd_D7_port |= (1<<lcd_D7_bit);     // make data = '1' if necessary
	lcd_D6_port &= ~(1<<lcd_D6_bit);                        // repeat for each data bit
	if (theByte & 1<<6) lcd_D6_port |= (1<<lcd_D6_bit);
	lcd_D5_port &= ~(1<<lcd_D5_bit);
	if (theByte & 1<<5) lcd_D5_port |= (1<<lcd_D5_bit);
	lcd_D4_port &= ~(1<<lcd_D4_bit);
	if (theByte & 1<<4) lcd_D4_port |= (1<<lcd_D4_bit);
	// write the data
	// 'Address set-up time' (40 nS)
	lcd_E_port |= (1<<lcd_E_bit);                   // Enable pin high
	_delay_us(70);                                   // implement 'Data set-up time' (80 nS) and 'Enable pulse width' (230 nS)
	lcd_E_port &= ~(1<<lcd_E_bit);                  // Enable pin low
	_delay_us(70);                                   // implement 'Data hold time' (10 nS) and 'Enable cycle time' (500 nS)
	}

void LCD_PrintLine(char *input_string, uint8_t line_number) {
	LCD_WriteInstruction4Bit(LCD_SET_CURSOR | line_number);
	LCD_WriteString4Bit(input_string);
}

void LCD_ClearLine(uint8_t line) {
	LCD_PrintLine("                    ", line);
}

void LCD_Clear() {
	LCD_ClearLine(LCD_LINE_1);
	LCD_ClearLine(LCD_LINE_2);
	LCD_ClearLine(LCD_LINE_3);
	LCD_ClearLine(LCD_LINE_4);
}

void LCD_PrintChar(uint8_t ch_in, uint8_t line, uint8_t position) {
	LCD_WriteInstruction4Bit((LCD_SET_CURSOR | line) + position);
	LCD_WriteCharacter4Bit(ch_in);
}
