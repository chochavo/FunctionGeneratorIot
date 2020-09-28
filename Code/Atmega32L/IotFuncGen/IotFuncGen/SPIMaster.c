#include "Headers\main.h"
#include "Headers\SPIMaster.h"
#include "Headers\USART.h"

void SPI_Write16Bit(uint8_t address_in, uint8_t data_in, enum Device device) {
	switch(device) {
		case DACA: SPI_CE_PORT &= ~DACA_NCE; break;
		case DACB: SPI_CE_PORT &= ~DACB_NCE; break;
		case DACA_BIAS: SPI_CE_PORT &= ~DACA_BIAS_NCE; break;
		case DACB_BIAS: SPI_CE_PORT &= ~DACB_BIAS_NCE; break;
		case FG0: SPI_CE_PORT &= ~FG0_NCE; break;
		case FG1: SPI_CE_PORT &= ~FG1_NCE; break;
		case LCD_POT: LCD_POT_PORT &= ~POT_LCD_NCE; break;
		default: break;
	}
	_delay_us(15);
	SPDR = address_in;
	while(!(SPSR & (1<<SPIF)));
	_delay_us(15);
	SPDR = data_in;
	while(!(SPSR & (1<<SPIF)));
	_delay_us(15);
	switch(device) {
		case DACA: SPI_CE_PORT |= DACA_NCE; break;
		case DACB: SPI_CE_PORT |= DACB_NCE; break;
		case DACA_BIAS: SPI_CE_PORT |= DACA_BIAS_NCE; break;
		case DACB_BIAS: SPI_CE_PORT |= DACB_BIAS_NCE; break;
		case FG0: SPI_CE_PORT |= FG0_NCE; break;
		case FG1: SPI_CE_PORT |= FG1_NCE; break;
		case LCD_POT: LCD_POT_PORT |= POT_LCD_NCE; break;
		default: break;
	}
}

void SPI_InitAD9834(void) {
	SPI_CE_PORT |= DACA_NCE | DACB_NCE | DACA_BIAS_NCE | DACB_BIAS_NCE | FG0_NCE | FG1_NCE;
	LCD_POT_PORT |= POT_LCD_NCE;
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<CPOL);
	SPSR = (1<<SPI2X);
	_delay_ms(10);
}

void SPI_InitAll(void) {
	SPI_CE_PORT |= DACA_NCE | DACB_NCE | DACA_BIAS_NCE | DACB_BIAS_NCE | FG0_NCE | FG1_NCE;
	LCD_POT_PORT |= POT_LCD_NCE;
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<CPOL);
	SPSR = (1<<SPI2X);
	_delay_ms(10);
}
