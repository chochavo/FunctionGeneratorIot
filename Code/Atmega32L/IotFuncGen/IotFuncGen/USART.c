#include "Headers\USART.h"
#include "Headers\main.h"

void Init_UART() {
	UBRRH = (uint8_t) (3 >> 8);
	UBRRL = (uint8_t) 3;
	UCSRB = (1 << TXEN);
	UCSRC = (1 << URSEL) | (1 << USBS) | (3 << UCSZ0);
}

void write_char_UART(char data) {
	while(!(UCSRA & (1<<UDRE)));
	UDR = data;
}

void Logger(char *str) {
	while (*str != '\0') {
		write_char_UART(*str);
		++str;
	}
}