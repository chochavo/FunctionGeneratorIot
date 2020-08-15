/*
 * USART.h
 *
 * Created: 1/5/2019 9:23:29 PM
 *  Author: MX
 */ 

#include "main.h"
#ifndef USART_H_
#define USART_H_

#define RX_BUFFER_LENGTH 128

struct uart{
	unsigned char rx_buffer[RX_BUFFER_LENGTH];
	volatile enum RX_STATES rx_state;
	volatile unsigned char rx_ptr;
	volatile bool message_received;
} uart;

/* Prototypes */
void Init_UART();
char UART_read_char();
void write_char_UART(char data);
void send_command_UART(char *str);
void clear_uart_rx_message();
#endif /* USART_H_ */