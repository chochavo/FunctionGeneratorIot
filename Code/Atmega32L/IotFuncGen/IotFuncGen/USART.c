#include "Headers\USART.h"
#include "Headers\main.h"

ISR(USART_RXC_vect, ISR_BLOCK) {
	switch(UART.wait_for_message) {
		case WAIT_FOR_OK:
			UART.rx_buffer[UART.rx_ptr] = UDR;
			if ((UART.rx_buffer[UART.rx_ptr] == '\n') && \
				(UART.rx_buffer[UART.rx_ptr - 2] == 'K') && \
				(UART.rx_buffer[UART.rx_ptr - 3] == 'O'))  {
				UART.message_received = 1;
				UART.rx_ptr = 0;
			}
			else UART.rx_ptr++;
			break;
					
		case WAIT_FOR_GOT_IP:
			UART.rx_buffer[UART.rx_ptr] = UDR;
			if ((UART.rx_buffer[UART.rx_ptr] == '\n') && \
				(UART.rx_buffer[UART.rx_ptr - 2] == 'P') && \
				(UART.rx_buffer[UART.rx_ptr - 3] == 'I'))  {
				UART.message_received = 1;
				UART.rx_ptr = 0;
			}
			else UART.rx_ptr++;
			break;
										
		case WAIT_FOR_READY_TO_SEND:
			UART.rx_buffer[UART.rx_ptr] = UDR;
			if ((UART.rx_buffer[UART.rx_ptr] == '>') && \
			(UART.rx_buffer[UART.rx_ptr - 1] == '\n') && \
			(UART.rx_buffer[UART.rx_ptr - 2] == '\r'))  {
				UART.message_received = 1;
				UART.rx_ptr = 0;
			}
			else UART.rx_ptr++;
			break;

		case WAIT_FOR_CONNECT:
			UART.rx_buffer[UART.rx_ptr] = UDR;
			if ((UART.rx_buffer[UART.rx_ptr] == '\n') && \
			(UART.rx_buffer[UART.rx_ptr - 2] == 'T') && \
			(UART.rx_buffer[UART.rx_ptr - 3] == 'C') && \
			(UART.rx_buffer[UART.rx_ptr - 4] == 'E')) {
				UART.message_received = 1;
				UART.rx_ptr = 0;
			}
			else UART.rx_ptr++;
			break;

		case WAIT_FOR_CLOSED:
			UART.rx_buffer[UART.rx_ptr] = UDR;
			if ((UART.rx_buffer[UART.rx_ptr] == '\n') && \
			(UART.rx_buffer[UART.rx_ptr - 2] == 'D') && \
			(UART.rx_buffer[UART.rx_ptr - 3] == 'E') && \
			(UART.rx_buffer[UART.rx_ptr - 4] == 'S')) {
				UART.message_received = 1;
				UART.rx_ptr = 0;
			}
			else UART.rx_ptr++;
			break;
								
		case WAIT_FOR_SEND_OK:
			UART.rx_buffer[UART.rx_ptr] = UDR;
			if ((UART.rx_buffer[UART.rx_ptr] == '\n') && \
			(UART.rx_buffer[UART.rx_ptr - 2] == 'K') && \
			(UART.rx_buffer[UART.rx_ptr - 3] == 'O') && \
			(UART.rx_buffer[UART.rx_ptr - 5] == 'D')) {
				UART.message_received = 1;
				UART.rx_ptr = 0;
			}
			else UART.rx_ptr++;
			break;
					
		case WAIT_FOR_READY:
			UART.rx_buffer[UART.rx_ptr] = UDR;
			if ((UART.rx_buffer[UART.rx_ptr] == '\n') && \
			(UART.rx_buffer[UART.rx_ptr - 2] == 'y') && \
			(UART.rx_buffer[UART.rx_ptr - 3] == 'd') && \
			(UART.rx_buffer[UART.rx_ptr - 4] == 'a')) {
				UART.message_received = 1;
				UART.rx_ptr = 0;
			}
			else UART.rx_ptr++;
			break;
					
		case WAIT_FOR_DATA: case WAIT_FOR_CREDENTIALS:
			UART.rx_buffer[UART.rx_ptr] = UDR;
			/* Strip \r\n+IPD... string to \r\nDATA\r\n */
			if (UART.cr_lf_counter == 0) {
				if ((UART.rx_buffer[UART.rx_ptr - 1] == '\r') && (UART.rx_buffer[UART.rx_ptr] == '\n')) {
					UART.cr_lf_counter++;
					UART.rx_ptr = 0;	
				}
				else UART.rx_ptr++;
			}
			else if (UART.cr_lf_counter == 1) {
				if ((UART.rx_buffer[UART.rx_ptr] == '\n') && (UART.rx_buffer[UART.rx_ptr - 1] == '\r')) {
					UART.rx_buffer[0] = '\r';
					UART.rx_buffer[1] = '\n';
					UART.cr_lf_counter++;
					UART.rx_ptr = 2;
				}
				else UART.rx_ptr++;
			}
			else if (UART.cr_lf_counter == 2) {
				if ((UART.rx_buffer[UART.rx_ptr] == '\n') && (UART.rx_buffer[UART.rx_ptr - 1] == '\r')) {
					UART.cr_lf_counter = 0;
					UART.rx_ptr = 0;
					UART.message_received = 1;
				}
				else UART.rx_ptr++;
			}
			break;
					
		case WAIT_FOR_MAC:
			UART.rx_buffer[UART.rx_ptr] = UDR;
			if (UART.rx_buffer[0] != '+') UART.rx_ptr = 0;
			else {
				if ((UART.rx_buffer[UART.rx_ptr - 1] == '\r') && (UART.rx_buffer[UART.rx_ptr] == '\n')) {
					UART.rx_ptr = 0;
					UART.message_received = 1;
				}
				else UART.rx_ptr++;
			}
			break;				
			
			default:
				UART.rx_ptr = 0;
				UART.message_received = 0;
				break;
	}
}

void Init_UART() {
	UBRRH = (unsigned char) (3 >> 8);
	UBRRL = (unsigned char) 3;
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
	UCSRC = (1 << URSEL) | (1 << USBS) | (3 << UCSZ0);
	clear_uart_rx_message();
	UART.cr_lf_counter = 0;
	UART.rx_ptr = 0;
	UART.wait_for_message = 0;
}

char UART_read_char() {
	while(!(UCSRA & (1<<RXC)));
	return UDR;
}

void write_char_UART(char data) {
	while(!(UCSRA & (1<<UDRE)));
	UDR=data;
}

void send_command_UART(char *str) {
	while (*str != '\0') {
		write_char_UART(*str);
		++str;
	}
}

void clear_uart_rx_message() {
	UART.message_received = false;
	memset(UART.rx_buffer,0,RX_BUFFER_LENGTH);
}
