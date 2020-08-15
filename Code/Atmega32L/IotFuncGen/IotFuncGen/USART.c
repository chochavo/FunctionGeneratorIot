#include "Headers\USART.h"
#include "Headers\main.h"


/*
struct {
	unsigned char rx_buffer[RX_BUFFER_LENGTH];
	enum RX_STATES rx_state;
	volatile unsigned char rx_ptr;
	volatile bool message_received;
} UART_module;
*/

enum RX_STATES {INIT = 1, ATE0_ACK, FIRST_CR_LF, REGULAR_DATA, TERMINATION};
static unsigned char temporary_buffer[RX_BUFFER_LENGTH];

ISR(USART_RXC_vect, ISR_BLOCK) {
	switch(uart.rx_state) {
		case INIT:
		
		temporary_buffer[0] = UDR;
		uart.rx_state = FIRST_CR_LF;
		uart.rx_ptr = 1;
		break;
		
		case FIRST_CR_LF:
		uart.rx_buffer[1] = UDR;
		if (uart.rx_buffer[0] == '\r' && uart.rx_buffer[1] == '\n') {
			uart.rx_state = REGULAR_DATA;
			uart.rx_ptr++;
		}
		else {
			uart.rx_state = INIT;
			uart.rx_ptr = 0;
			uart.rx_buffer[0] = 0; uart.rx_buffer[1] = 0;
		}
		break;
		
		case REGULAR_DATA:
		uart.rx_buffer[uart.rx_ptr] = UDR;
		if (uart.rx_buffer[uart.rx_ptr] == '\r') uart.rx_state = TERMINATION;
		uart.rx_ptr++;
		break;
		
		case TERMINATION:
		uart.rx_buffer[uart.rx_ptr] = UDR;
		if (uart.rx_buffer[uart.rx_ptr - 1] == '\r' && uart.rx_buffer[uart.rx_ptr] == '\n') {
			uart.rx_state = INIT;
			uart.rx_ptr = 0;
			uart.message_received = true;
		}
		else {
			uart.rx_ptr++;
			uart.rx_state = REGULAR_DATA;
		}
		break;
		
		default: break;
	}
}

void Init_UART()
{
	UBRRH = (unsigned char) (3 >> 8);
	UBRRL = (unsigned char) 3;
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
	UCSRC = (1 << URSEL) | (1 << USBS) | (3 << UCSZ0);
	uart.rx_state = INIT;
}

char UART_read_char()
{
	while(!(UCSRA & (1<<RXC)));
	return UDR;
}

void write_char_UART(char data)
{
	while(!(UCSRA & (1<<UDRE)));
	UDR=data;
}

void send_command_UART(char *str)
{
	while (*str != '\0')
	{
		write_char_UART(*str);
		++str;
	}
}

void clear_uart_rx_message() {
	uart.message_received = false;
	memset(uart.rx_buffer,0,RX_BUFFER_LENGTH);
	memset(temporary_buffer, 0, RX_BUFFER_LENGTH);
}

/*
unsigned char UART_read_string(unsigned char size)
{
	unsigned char i = 0;

	if (size == 0) return 0;            // return 0 if no space

	while (i < size - 1) {              // check space is available (including additional null char at end)
		unsigned char c;
		while ( !(UCSRA & (1<<RXC)) );  // wait for another char - WARNING this will wait forever if nothing is received
		c = UDR;
		if (c == '\n') break;           // break on NULL character
		rx_buffer[i] = c;                       // write into the supplied buffer
		i++;
	}
	rx_buffer[i] = '\n';                           // ensure string is null terminated

	return i + 1;                       // return number of characters written
}
*/