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

enum UART_RX_MESSAGE_TYPES { WAIT_FOR_OK = 1, WAIT_FOR_GOT_IP, WAIT_FOR_READY_TO_SEND,
	WAIT_FOR_CONNECT, WAIT_FOR_CLOSED, WAIT_FOR_SEND_OK,
WAIT_FOR_READY, WAIT_FOR_DATA ,WAIT_FOR_CREDENTIALS ,WAIT_FOR_MAC };
	
struct UART{
	unsigned char rx_buffer[RX_BUFFER_LENGTH];
	volatile unsigned char rx_ptr;
	volatile bool message_received;
	volatile unsigned char cr_lf_counter;
	volatile enum UART_RX_MESSAGE_TYPES wait_for_message;
} UART;

/* Prototypes */
void Init_UART();
char UART_read_char();
void write_char_UART(unsigned char data);
void send_command_UART(unsigned char *str);
void clear_uart_rx_message();
#endif /* USART_H_ */