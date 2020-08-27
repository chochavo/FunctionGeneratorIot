/*
 * USART.h
 *
 * Created: 1/5/2019 9:23:29 PM
 *  Author: MX
 */ 

#include "main.h"
#ifndef USART_H_
#define USART_H_

/* Prototypes */
void Init_UART();
char UART_read_char();
void write_char_UART(char data);
void Logger(char *message);
#endif /* USART_H_ */