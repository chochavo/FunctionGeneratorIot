/*
 * IoT Dual Channel Function Generator V1.4
 * Designed by: Michael Khomyakov
 * July 2019
 * Department of Electric and Electronic Engineering
 * Ariel University
 */ 
//#include "main.h"
#include "Headers\main.h"
#include "Headers\SPIMaster.h"
#include "Headers\USART.h"
#include "Headers\LCD.h"
#include "Headers\WifiParams.h"

//volatile bool isr_executed = false;

//ISR(BADISR_vect) { sei(); }
	
//ISR(TIMER1_COMPA_vect) {
	//DISABLE_TIMER();
	//TCNT1 = 0;
	//isr_executed = true;
//}

void update_device_status() { // TODO: Future use of timer interrupt for AC / Battery settings
// 	if (isr_executed) {
// 		volatile bool prev_ac_status = status.ac_power_status;
// 		update_ac_power_status();
// 		update_battery_status();
// 		if (status.ac_power_status && !prev_ac_status) {
// 			beep();
// 			clear_LCD();
// 			print_LCD_line("AC connected!       ", LCD_LINE_2);
// 			_delay_ms(1000);
// 			update_complete_UI();
// 		}
// 		else if (!status.ac_power_status && prev_ac_status) {
// 			beep();
// 			clear_LCD();
// 			print_LCD_line("AC disconnected!    ", LCD_LINE_2);
// 			_delay_ms(1000);
// 			update_complete_UI();
// 			//ENABLE_TIMER();
// 		}
// 		if (status.battery_voltage <= 50) {
// 			clear_LCD();
// 			print_LCD_line("Battery low-energy! ", LCD_LINE_2);
// 			_delay_ms(1000);
// 			shutdown_sequence(false);
// 		}
// 		if (poll_switch()) shutdown_sequence(false);
// 		isr_executed = false;
// //		ENABLE_TIMER();
// 	}
}



void beep() { // d - f - a
		for (uint8_t ptrm = 0; ptrm < 70; ptrm++) {
			MISC_PORT |= BUZZER;
			_delay_us(426);
			MISC_PORT &= ~BUZZER;
			_delay_us(426);
		}
	
}

void play_melody(bool power_on) {
	if (power_on) {
		for (uint8_t ptrm = 0; ptrm < 236; ptrm++) {
			MISC_PORT |= BUZZER;
			_delay_us(426);
			MISC_PORT &= ~BUZZER;
			_delay_us(426);
		}
		for (uint16_t ptrm = 0; ptrm < 296; ptrm++) {
			MISC_PORT |= BUZZER;
			_delay_us(338);
			MISC_PORT &= ~BUZZER;
			_delay_us(338);
		}
		for (uint16_t ptrm = 0; ptrm < 704; ptrm++) {
			MISC_PORT |= BUZZER;
			_delay_us(284);
			MISC_PORT &= ~BUZZER;
			_delay_us(284);
		}
	}
	else {
		for (uint16_t ptrm = 0; ptrm < 296; ptrm++) {
			MISC_PORT |= BUZZER;
			_delay_us(338);
			MISC_PORT &= ~BUZZER;
			_delay_us(338);
		}
		for (uint8_t ptrm = 0; ptrm < 236; ptrm++) {
			MISC_PORT |= BUZZER;
			_delay_us(426);
			MISC_PORT &= ~BUZZER;
			_delay_us(426);
		}
		for (uint16_t ptrm = 0; ptrm < 352; ptrm++) {
			MISC_PORT |= BUZZER;
			_delay_us(568);
			MISC_PORT &= ~BUZZER;
			_delay_us(568);
		}
	}
}

void shutdown_sequence(bool is_erase_requested) {
	beep();
	clear_LCD();
	print_LCD_line("<<Shutdown request>>", LCD_LINE_1);
	print_LCD_line("Device shutting down", LCD_LINE_2);
	print_LCD_line("in X sec            ", LCD_LINE_3);
	for (uint8_t cntx = 5; cntx > 0; cntx--) {
		print_LCD_char(cntx + '0',LCD_LINE_3, 3);
		_delay_ms(DELAY_COMMAND_MS);
	}
	//send_command_UART("SHDN\r\n");
	if (is_erase_requested) erase_EEPROM_1K();
	play_melody(false);
	DISABLE_DEVICE();
}

void Init_Timer() {
	TIMSK |= (1 << OCIE1A);
	TCNT1 = 0;
	TCCR1B |= (1 << WGM12);
	OCR1A = 1000;
	//ENABLE_TIMER();
}

void Init_Ports() {
	SPI_CE_DDR |= DACA_NCE | DACB_NCE | DACA_BIAS_NCE | DACB_BIAS_NCE | FG0_NCE | FG1_NCE;
	LCD_POT_DDR |= POT_LCD_NCE;
	LED_DDR |= LED_R | LED_G | LED_B;
	LCD_CONTROL_DDR |= LCD_RS | LCD_E;
	LCD_DATA_DDR |= LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7;
	ADC_DDR &= ~VBAT_ADC & ~PWR_IND;
	SPI_DDR |= SPI_MOSI | SPI_SCK;
	SPI_DDR &= ~SPI_MISO;
	UART_DDR |= UART_TX;
	UART_DDR &= ~UART_RX;
	FG_SEL_DDR |= FG0_SEL | FG1_SEL;
	PB_DDR &= ~S_INT;
	MISC_DDR |= PS_HOLD | BUZZER;
}

void Init_Device() {
	Init_Ports();
	ENABLE_DEVICE();
	Init_SPI_All();
	Init_LCD();
	_delay_ms(DELAY_COMMAND_MS);
	Init_LCD_4bit();
	Init_UART();
	Init_ADC();
	clear_all_values();
	STATUS.socket_active = false;
}

void Init_ADC() {
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);
	ADMUX |= (1 << REFS0);
}

void Init_UI() {
	set_animated_brightness();
	set_LCD_contrast(100);
	#if RGB_LED_SUPPORT
	set_LED(RED);
	#endif
	#if SKIP_LOGO
	#else
	LCD_logo_display();
	play_melody(true);
	_delay_ms(2 * DELAY_COMMAND_MS);
	#endif
}

void set_output_selection(enum DEVICES device, enum MODES device_mode) {
	switch(device_mode) {
		case SQUARE:
			if (device == FG0) FG_SEL_PORT &= ~FG0_SEL;
			else if (device == FG1) FG_SEL_PORT &= ~FG1_SEL;
			break;
		default:
			if (device == FG0) FG_SEL_PORT |= FG0_SEL;
			else if (device == FG1) FG_SEL_PORT |= FG1_SEL;
			break;			
	}
}

void set_functionality(enum DEVICES device, uint32_t device_freq, enum MODES mode) {
	Init_SPI_AD9834();
	device_freq *= AD9834_FREQ_FACTOR;
	uint16_t freq_reg_lsb = device_freq & 0x0003FFF;
	uint16_t freq_reg_msb = ((device_freq >> 14) & 0x0003FFF);
	uint8_t freq_reg_lsb_a = ((freq_reg_lsb >> 8) & 0x3F);
	uint8_t freq_reg_lsb_b = (freq_reg_lsb & 0xFF);
	uint8_t freq_reg_msb_a = ((freq_reg_msb >> 8) & 0x3F);
	uint8_t freq_reg_msb_b = (freq_reg_msb & 0xFF);
		
	SPI_write_16bit(AD9834_CONSECUTIVE_WRITE,0x00,device);
	SPI_write_16bit(AD9834_FREQUENCY_REGISTER_ADDR |freq_reg_lsb_a, freq_reg_lsb_b, device);
	SPI_write_16bit(AD9834_FREQUENCY_REGISTER_ADDR |freq_reg_msb_a, freq_reg_msb_b, device);
	SPI_write_16bit(AD9834_PHASE_REGISTER_ADDR ,0x00, device);
	switch(mode) {
		case SINE: SPI_write_16bit(AD9834_EXIT_RESET ,OPBITEN, device); break;
		case TRIANGLE: SPI_write_16bit(AD9834_EXIT_RESET, MODE, device); break;
		case SQUARE: SPI_write_16bit(AD9834_EXIT_RESET, OPBITEN | DIV2, device); break;
		default: SPI_write_16bit(AD9834_EXIT_RESET, SLEEP1, device); break;
		}
		set_output_selection(device, mode);
	Init_SPI_All();
}

int set_amplitude(uint16_t val_in, enum DEVICES device) {
	uint16_t value = 4095 - val_in;
	uint8_t dac_msb = ((value >> 8) & 0x0F);
	uint8_t dac_lsb = value & 0xFF;
	if (device == FG0)		SPI_write_16bit(AMPLITUDE_A_ADDR | dac_msb, dac_lsb, DACA);
	else if (device == FG1) SPI_write_16bit(AMPLITUDE_B_ADDR | dac_msb, dac_lsb, DACB);
	else return -1; 
	return 0;
}

#if RGB_LED_SUPPORT
void set_LED(enum LED_STATES LED) {
	switch(LED) {
		case RED: LED_PORT |= LED_R; LED_PORT &= ~LED_G & ~LED_B; break;
		case GREEN: LED_PORT &= ~LED_R & ~LED_B; LED_PORT |= LED_G;  break;
		case BLUE: LED_PORT &= ~LED_R & ~LED_G; LED_PORT |= LED_B; break;
		case LED_OFF: LED_PORT &= ~LED_R & ~LED_G & ~LED_B; break;
		default: break;
	}
}
#endif

bool poll_switch() {
	if (!(PB_PIN & S_INT)) {
		while(!(PB_PIN & S_INT));
		return true; 
	}
	else return false; 	
}

void test_LEDS() {
	set_LED(RED);
	_delay_ms(500);
	set_LED(GREEN);
	_delay_ms(500);
	set_LED(BLUE);
	_delay_ms(500);
	set_LED(RED);
}

void set_animated_brightness() {
	for (uint8_t ix = 0; ix < 101; ix = ix + 1) {
		set_LCD_brightness(ix);
		_delay_ms(10);
	}
}

void set_LCD_contrast(uint8_t value) {
	uint8_t final_value = (value * MAX_8BIT) / 100;
	SPI_write_16bit(CONTRAST_ADDR,255 - final_value,LCD_POT);
}


int set_LCD_brightness(uint8_t value) {
	if (value > 100) return -1;
	value = 75 + ((value * 25) / 100);
	uint16_t transformed_val = (value * DAC_DC_RATIO);
	uint8_t byte_a = (transformed_val >> 8) & 0x0F;
	uint8_t byte_b = transformed_val & 0xFF; 
	SPI_write_16bit(VOLUME_ADDR | byte_a ,byte_b ,DACB);
	return 0;
}

int set_dc_bias (enum DEVICES device, uint16_t value, bool sign) {
	uint8_t reg_msb = (value >> 8) & 0x0F;
	uint8_t reg_lsb = value & 0xFF;
		switch(device) {
			case FG0:
				if (sign) SPI_write_16bit(BIAS_A_NEG_ADDR | reg_msb, reg_lsb, DACA_BIAS);
				else	  SPI_write_16bit(BIAS_A_POS_ADDR | reg_msb, reg_lsb, DACA_BIAS);
				break;
			case FG1:
				if (sign) SPI_write_16bit(BIAS_B_NEG_ADDR | reg_msb, reg_lsb, DACB_BIAS);
				else	  SPI_write_16bit(BIAS_B_POS_ADDR | reg_msb, reg_lsb, DACB_BIAS);
				break;
			default: return -1;	// Wrong device
				break;
		}
	return 0;
	}

void erase_EEPROM_1K() {
	for (uint16_t ptr = 0; ptr < EEPROM_ADRESS_SPAN + 1; ptr++) eeprom_write_byte((uint8_t *)ptr, 0);
}

char * create_wifi_command() {
	int sprintf_store = 0;
	static char command[MAX_COMMAND_LENGTH];
	memset(command, 0, MAX_COMMAND_LENGTH);
	memset(WIFI.SSID, 0, MAX_WIFI_SSID_LENGTH);
	memset(WIFI.PASS, 0, MAX_WIFI_PASS_LENGTH);
	WIFI.encryption = 0;
	eeprom_read_block(WIFI.SSID, (uint8_t *)WIFI_SSID_ADDRESS, MAX_WIFI_SSID_LENGTH);
	eeprom_read_block(WIFI.PASS, (uint8_t *)WIFI_PASS_ADDRESS, MAX_WIFI_PASS_LENGTH);
	WIFI.encryption = eeprom_read_byte(( uint8_t *)WIFI_ENCRYPTION_ADDRESS);
	sprintf_store = sprintf(command, "AT+CWJAP=\"%s\",\"%s\"\r\n",WIFI.SSID,WIFI.PASS);
	return command;
}

void save_wifi_credentials() {
	eeprom_write_block(WIFI.SSID, (uint8_t *)WIFI_SSID_ADDRESS, MAX_WIFI_SSID_LENGTH);
	eeprom_write_block(WIFI.PASS, (uint8_t *)WIFI_PASS_ADDRESS, MAX_WIFI_PASS_LENGTH);
	eeprom_write_byte(( uint8_t *)WIFI_ENCRYPTION_ADDRESS, WIFI.encryption);
}

void retrieve_wifi_credentials() {
	memset(WIFI.SSID, 0, MAX_WIFI_SSID_LENGTH);
	memset(WIFI.PASS, 0, MAX_WIFI_PASS_LENGTH);
	volatile uint8_t end_of_ssid_ptr,start_of_pass_ptr,end_of_pass_ptr = 0;
	for (uint8_t ptr = START_OF_SSID_PTR; ptr < MAX_WIFI_SSID_LENGTH + START_OF_SSID_PTR; ptr++) {
		if (UART.rx_buffer[ptr] == ',' && UART.rx_buffer[ptr + 1] == 'P') { 
			end_of_ssid_ptr = ptr - 1;
			start_of_pass_ptr = ptr + 2;
			break;
		}
	}
	for (uint8_t ptr = start_of_pass_ptr; ptr < start_of_pass_ptr + MAX_WIFI_PASS_LENGTH; ptr++) {
		if (UART.rx_buffer[ptr] == ',' && UART.rx_buffer[ptr + 1] == 'E') {
			end_of_pass_ptr = ptr - 1;
			break;
		}
	}
	memcpy(WIFI.SSID, UART.rx_buffer + START_OF_SSID_PTR, end_of_ssid_ptr - START_OF_SSID_PTR + 1);
	memcpy(WIFI.PASS, UART.rx_buffer + start_of_pass_ptr, end_of_pass_ptr - start_of_pass_ptr + 1);
	WIFI.encryption = UART.rx_buffer[end_of_pass_ptr + 3];
}

void pack_mac_string() {
	memcpy(WIFI.device_MAC, "MAC=", 4);
	memcpy(WIFI.device_MAC + 4, UART.rx_buffer + 12, 17);
	memcpy(WIFI.device_MAC + 21, "\r\n", 2);
}

void halt_system() {
	clear_LCD();
	print_LCD_line("Communication error!", LCD_LINE_1);
	print_LCD_line("Press the button    ", LCD_LINE_2);
	print_LCD_line("to restart device   ", LCD_LINE_3);
	print_LCD_line("   <<< ERROR >>>    ", LCD_LINE_4);
	while(true) {
		if (poll_switch()) {
			send_command_UART("AT+GLSP=1\r\n"); // Perform reset
			_delay_ms(DELAY_COMMAND_MS);
			RESET_DEVICE();
		}
	}
}

void direct_pairing() {
	uint8_t retries = 0;
	bool break_from_pairing = false;
	enum DIRECT_PAIRING_STATES direct_pairing_state = SHOW_DIRECT_MESSAGE;
	beep();
	while(!break_from_pairing) {
		
		if (poll_switch()) shutdown_sequence(true);
		
		if (retries < MAXIMUM_COMMAND_RETRIES) {
			switch(direct_pairing_state)
			{
				case SHOW_DIRECT_MESSAGE:
					clear_LCD();
					print_LCD_line(ESP32_STATUS_MSG, LCD_LINE_1);
					direct_pairing_state = ECHO_OFF_COMMAND;
					UART.wait_for_message = WAIT_FOR_OK;
					break;
			
				case ECHO_OFF_COMMAND: // create wifi AP, create socket.
					if (UART.message_received) {
							direct_pairing_state = SET_AP_MODE;
							clear_uart_rx_message();
							retries = 0;
							UART.wait_for_message = WAIT_FOR_OK;
							print_LCD_line(ESP32_STATUS_MSG_OK, LCD_LINE_1);				
							print_LCD_line(REQUEST_NETWORK_MSG, LCD_LINE_2);
						}
					else {
						retries++;
						send_command_UART("ATE0\r\n");
						_delay_ms(DELAY_COMMAND_MS);
					}
					break;
			
				case SET_AP_MODE: // create wifi AP, create socket.
					if (UART.message_received) {
							direct_pairing_state = OPEN_DIRECT_AP;
							clear_uart_rx_message();
							retries = 0;
							UART.wait_for_message = WAIT_FOR_OK;
							break;
						}
					//else clear_uart_rx_message();
					retries++;
					send_command_UART("AT+CWMODE=3\r\n");
					_delay_ms(DELAY_COMMAND_MS);
					break;
							
				case OPEN_DIRECT_AP:
					if (UART.message_received) {
						direct_pairing_state = SET_MUX_COMMAND;
						clear_uart_rx_message();
						retries = 0;
						UART.wait_for_message = WAIT_FOR_OK;
						print_LCD_line(REQUEST_NETWORK_MSG_OK, LCD_LINE_2);
						print_LCD_line(CREATING_SERVER_MSG, LCD_LINE_3);
						break;
						}
					
					retries++;
					send_command_UART("AT+CWSAP=\"IOT_FUNCGEN\",\"0\",1,0\r\n");
					_delay_ms(DELAY_COMMAND_MS);
					break;
				
				case SET_MUX_COMMAND:
					if (UART.message_received) {
							direct_pairing_state = OPEN_SOCKET_SERVER;
							clear_uart_rx_message();
							retries = 0;
							UART.wait_for_message = WAIT_FOR_OK;
							break;
					}
					retries++;
					send_command_UART("AT+CIPMUX=1\r\n");
					_delay_ms(DELAY_COMMAND_MS);
					break;	
					
				case OPEN_SOCKET_SERVER:
					if (UART.message_received) {
							direct_pairing_state = WAIT_FOR_DEVICE;
							clear_uart_rx_message();
							retries = 0;
							print_LCD_line(CREATING_SERVER_MSG_OK, LCD_LINE_3);
							print_LCD_line(WAIT_FOR_DEVICE_MSG, LCD_LINE_4);
							UART.wait_for_message = WAIT_FOR_CONNECT;
							break;
					}
					retries++;
					send_command_UART("AT+CIPSERVER=1,1726\r\n");
					_delay_ms(DELAY_COMMAND_MS);
					break;
				
				
				case WAIT_FOR_DEVICE:
					if (UART.message_received) {
							direct_pairing_state = MESSAGE_HANDLING_STATE;
							UART.wait_for_message = WAIT_FOR_DATA;
							beep(); _delay_ms(50); beep(); _delay_ms(50); beep();
							clear_uart_rx_message();
							clear_LCD();
							retries = 0;
							print_LCD_line(DEVICE_CONNECTED_MSG, LCD_LINE_2);
							_delay_ms(2 * DELAY_COMMAND_MS);
							break;
					}
					break;				
				
				case MESSAGE_HANDLING_STATE:
					eeprom_write_byte((uint8_t *)STORED_CONNECTION_STATE_ADDRESS, WIFI_DIRECT_CONNECTION);
					update_complete_UI();
					socket_message_handler();	
					break;		
			}	
		}
		else {
			halt_system();
			retries = 0;
			direct_pairing_state = SHOW_DIRECT_MESSAGE;
			_delay_ms(DELAY_COMMAND_MS);
		}
	}
}

void wifi_lan_pairing() {
	uint8_t retries = 0;
	bool break_from_pairing = false;
	enum LAN_PAIRING_STATES pairing_state = SHOW_LAN_MESSAGE;
	char buffer_LCD1[LCD_LINE_LENGTH];
	char buffer_LCD2[LCD_LINE_LENGTH];
	while(!break_from_pairing) {
		if (retries < MAXIMUM_COMMAND_RETRIES) {
			switch(pairing_state)
			{
				case SHOW_LAN_MESSAGE:
					clear_LCD();
					print_LCD_line(ESP32_STATUS_MSG, LCD_LINE_1);
					pairing_state = ECHO_OFF_COMMAND;
					UART.wait_for_message = WAIT_FOR_OK;
					break;
				
				case ECHO_OFF: // create wifi AP, create socket.
				if (UART.message_received) {
						pairing_state = OPEN_DIRECT_AP;
						clear_uart_rx_message();
						retries = 0;
						print_LCD_line(ESP32_STATUS_MSG_OK, LCD_LINE_1);
						print_LCD_line(REQUEST_NETWORK_MSG, LCD_LINE_2);
						UART.wait_for_message = WAIT_FOR_OK;
						break;
				}
				send_command_UART("ATE0\r\n");
				retries++;
				_delay_ms(DELAY_COMMAND_MS);
				if (poll_switch()) shutdown_sequence(true);
				break;
			
				case SET_AP_MODE: // create wifi AP, create socket.
					if (UART.message_received) {
							pairing_state = OPEN_LOCAL_AP;
							clear_uart_rx_message();
							retries = 0;
							UART.wait_for_message = WAIT_FOR_OK;
							break;
					}
					retries++;
					send_command_UART("AT+CWMODE=1\r\n");
					_delay_ms(DELAY_COMMAND_MS);
					if (poll_switch()) shutdown_sequence(true);
					break;
							
				case OPEN_LOCAL_AP:
					if (UART.message_received) {
						pairing_state = SET_MUX_COMMAND;
						clear_uart_rx_message();
						retries = 0;
						print_LCD_line(REQUEST_NETWORK_MSG_OK, LCD_LINE_2);
						print_LCD_line(CREATING_SERVER_MSG, LCD_LINE_3);
						UART.wait_for_message = WAIT_FOR_OK;
						break;
					}
					send_command_UART("AT+CWSAP=\"IOT_FUNCGEN\",\"0\",1,0\r\n");
					retries++;
					_delay_ms(DELAY_COMMAND_MS);
					if (poll_switch()) shutdown_sequence(true);
					break;
				
				case SET_MUX_COMMAND:
					if (UART.message_received) {
						pairing_state = OPEN_LOCAL_SOCKET;
						UART.wait_for_message = WAIT_FOR_OK;
						clear_uart_rx_message();
						retries = 0;
						break;
					}
					retries++;
					send_command_UART("AT+CIPMUX=1\r\n");
					_delay_ms(DELAY_COMMAND_MS);
					if (poll_switch()) shutdown_sequence(true);
					break;
								
				case OPEN_LOCAL_SOCKET:
					if (UART.message_received) {
						pairing_state = WAIT_FOR_DEVICE_CONNECT;
						UART.wait_for_message = WAIT_FOR_CONNECT;
						clear_uart_rx_message();
						retries = 0;
						print_LCD_line(CREATING_SERVER_MSG_OK, LCD_LINE_3);
						print_LCD_line(WAIT_FOR_DEVICE_MSG, LCD_LINE_4);
						break;
					}
					send_command_UART("AT+CIPSERVER=1,1726\r\n");
					retries++;
					_delay_ms(DELAY_COMMAND_MS);
					if (poll_switch()) shutdown_sequence(true);
					break;
				
				case WAIT_FOR_DEVICE_CONNECT:
					if (UART.message_received) {
						pairing_state = RETRIEVE_MAC;
						UART.wait_for_message = WAIT_FOR_MAC;
						clear_uart_rx_message();
						clear_LCD();
						print_LCD_line("Android device found", LCD_LINE_1);
						print_LCD_line("FuncGen MAC Address:", LCD_LINE_2);
						print_LCD_line("<   Retrieving...  >", LCD_LINE_3);
						STATUS.socket_active = true;
						retries = 0;
						_delay_ms(DELAY_COMMAND_MS);
						break;
					}
					if (poll_switch()) shutdown_sequence(true);
					break;
				
				case RETRIEVE_MAC:
					if (UART.message_received) {
						pack_mac_string();
						print_LCD_line(WIFI.device_MAC, LCD_LINE_3);
						_delay_ms(1000);
						pairing_state = SEND_MAC;
						UART.wait_for_message = WAIT_FOR_READY_TO_SEND;
						clear_uart_rx_message();
						retries = 0;
					}
					send_command_UART("AT+CIPSTAMAC?\r\n");
					retries++;
					_delay_ms(DELAY_COMMAND_MS);					
					if (poll_switch()) shutdown_sequence(true);
					break;
				
				case SEND_MAC:
					while(true) {
						if (poll_switch()) shutdown_sequence(true);
						if (UART.message_received) {
							clear_uart_rx_message();
							retries = 0;
							UART.wait_for_message = WAIT_FOR_CREDENTIALS;
							break;
						}
						else {
							if (retries >= MAXIMUM_COMMAND_RETRIES) {
								shutdown_sequence(true);
							}
							else {
								send_command_UART("AT+CIPSEND=0,23\r\n");
								retries++;
								_delay_ms(DELAY_COMMAND_MS);
							}
						}
					}
					send_command_UART(WIFI.device_MAC); // MAC String transmission
					while(true) {
						if (poll_switch()) shutdown_sequence(true);
						if (UART.message_received) {
							pairing_state = RETRIEVE_CREDENTIALS;
							retries = 0;
							break;
						}
						else {
							if (retries >= MAXIMUM_COMMAND_RETRIES) {
								shutdown_sequence(true);
							}
							else {
								retries++;
								_delay_ms(DELAY_COMMAND_MS);
							}
						}
					}
					break;

				case RETRIEVE_CREDENTIALS: //\r\nWN=SVaica1,Pvaica666,E2\r\n
					if (UART.message_received) {
						clear_LCD();
						retrieve_wifi_credentials();
						save_wifi_credentials();
						memset(buffer_LCD1, 0, LCD_LINE_LENGTH);
						memset(buffer_LCD2, 0, LCD_LINE_LENGTH);
						snprintf(buffer_LCD1, MAX_STRING_BUFFER, "SSID:%s", WIFI.SSID);
						snprintf(buffer_LCD2, MAX_STRING_BUFFER, "PASS:%s", WIFI.PASS);
						print_LCD_line("Wi-Fi data received:", LCD_LINE_1);
						retries = 0;
						print_LCD_line(buffer_LCD1, LCD_LINE_2);
						print_LCD_line(buffer_LCD2, LCD_LINE_3);
						if (WIFI.encryption != '4') print_LCD_line("Connection: Secured", LCD_LINE_4);
						else  print_LCD_line("Connection: Open    ", LCD_LINE_4);
						_delay_ms(4 * DELAY_COMMAND_MS);
						clear_uart_rx_message();
						start_wlan_communication();
						break;
					}
					if (poll_switch()) shutdown_sequence(true);
			}
		}
		else {
			halt_system();
			retries = 0;
			pairing_state = SHOW_LAN_MESSAGE;
			_delay_ms(1000);
		}
	}
}

void start_wlan_communication() {
	uint8_t retries = 0;
	bool break_from_pairing = false;
	enum WLAN_COMMUNICATION_STATES communication_state = INIT_MESSAGE;
	beep();
	while(!break_from_pairing) {
		if (retries < MAXIMUM_COMMAND_RETRIES) {
			switch(communication_state) {	
				case INIT_MESSAGE:
				clear_LCD();
				print_LCD_line(START_WLAN_MSG, LCD_LINE_2);
				print_LCD_line(START_COMM_MSG, LCD_LINE_3);
				retries = 0;
				_delay_ms(2 * DELAY_COMMAND_MS);
				clear_LCD();
				print_LCD_line(ESP32_STATUS_MSG, LCD_LINE_1);
				communication_state = ECHO_OFF;
				UART.wait_for_message = WAIT_FOR_OK;
				break;
				
				case ECHO_OFF: // create wifi AP, create socket.
					if (UART.message_received) {
						communication_state = REQUEST_WIFI;
						clear_uart_rx_message();
						retries = 0;
						UART.wait_for_message = WAIT_FOR_GOT_IP;
						print_LCD_line(ESP32_STATUS_MSG_OK, LCD_LINE_1);
						print_LCD_line(REQUEST_NETWORK_MSG, LCD_LINE_2);
						break;
					}
					send_command_UART("ATE0\r\n");
					retries++;
					_delay_ms(DELAY_COMMAND_MS);
					if (poll_switch()) shutdown_sequence(true);
					break;

				case REQUEST_WIFI: // create wifi AP, create socket.
					if (UART.message_received) {
						communication_state = CREATE_SOCKET_SERVER_WLAN;
						UART.wait_for_message = WAIT_FOR_OK;
						clear_uart_rx_message();
						print_LCD_line(REQUEST_NETWORK_MSG_OK, LCD_LINE_2);
						print_LCD_line(CREATING_SERVER_MSG, LCD_LINE_3);
						retries = 0;
						break;
					}
					send_command_UART(create_wifi_command());
					retries++;
					_delay_ms(DELAY_COMMAND_MS);
					if (poll_switch()) shutdown_sequence(true);
					break;
				
				case CREATE_SOCKET_SERVER_WLAN: // create wifi AP, create socket.
					if (UART.message_received) {
						communication_state = WAIT_FOR_WLAN_DEVICE;
						UART.wait_for_message = WAIT_FOR_CONNECT;
						clear_uart_rx_message();
						print_LCD_line(CREATING_SERVER_MSG_OK, LCD_LINE_3);
						print_LCD_line(WAIT_FOR_DEVICE_MSG, LCD_LINE_4);
						retries = 0;
						clear_LCD();
						break;
					}
					send_command_UART("AT+CIPSERVER=1,1726\r\n");
					retries++;
					_delay_ms(DELAY_COMMAND_MS);
					if (poll_switch()) shutdown_sequence(true);
					break;
				
				case WAIT_FOR_WLAN_DEVICE:
					if (UART.message_received) {
						communication_state = DEVICE_CONNECTED;
						UART.wait_for_message = WAIT_FOR_DATA;
						clear_uart_rx_message();
						retries = 0;
						clear_LCD();
						print_LCD_line(DEVICE_CONNECTED_MSG, LCD_LINE_2);
						STATUS.socket_active = true;
						beep(); _delay_ms(50); beep();
						_delay_ms(2 * DELAY_COMMAND_MS);
						break;
					}
					if (poll_switch()) shutdown_sequence(true);
					break;

				case DEVICE_CONNECTED:
					eeprom_write_byte((uint8_t *)STORED_CONNECTION_STATE_ADDRESS, WIFI_LAN_CONNECTON);
					init_UI_array();
					update_complete_UI();
					socket_message_handler();
					break;
				
			}
		}
		else {
			halt_system();
			retries = 0;
			communication_state = INIT_MESSAGE;
			_delay_ms(DELAY_COMMAND_MS);
		}
	}
}


void socket_message_handler() {
	volatile enum MESSAGE_HANDLER_STATES msg_state = MESSAGE_HANDLER_IDLE;
	bool output_confirm = false;
	//Init_Timer();
	while(!output_confirm) {
		update_device_status();
		switch(msg_state) {
			case MESSAGE_HANDLER_IDLE:
				if (UART.message_received) {
//					DISABLE_TIMER();
					beep();
					if ((UART.rx_buffer[2]) == 'G' && (UART.rx_buffer[3] == 'I')) msg_state = PROVIDE_DETAILS;
					else if ((UART.rx_buffer[2]) == 'B' && (UART.rx_buffer[3] == 'O')) msg_state = BOOT_FROM_APP;
					else if (UART.rx_buffer[2] == 'C' && UART.rx_buffer[3] == '=') msg_state = FUNCTION_HANDLER;
					else if (UART.rx_buffer[2] == 'L' && UART.rx_buffer[3] == 'C') msg_state = LCD_PARAM_ADJUST;
					else if (UART.rx_buffer[2] == 'R' && UART.rx_buffer[3] == 'E') msg_state = RESET_HANDLER;
					else clear_uart_rx_message();
				}
				break;
				
			case PROVIDE_DETAILS:
				switch(UART.rx_buffer[7]) {
					case 'B': send_command_UART(get_battery_status());  break;
					case 'A':  send_command_UART(get_ac_power_status()); break;
					default: break;
				}
				clear_uart_rx_message();
				msg_state = MESSAGE_HANDLER_IDLE;
//				ENABLE_TIMER();
				break;

			case FUNCTION_HANDLER:
				set_parameter();
				clear_uart_rx_message();
				msg_state = MESSAGE_HANDLER_IDLE;
				//send_command_UART("OK\r\n");
//				ENABLE_TIMER();
				break;
				
			case RESET_HANDLER:
				clear_LCD();
				print_LCD_line(SHUTDOWN_MSG, LCD_LINE_2);
				print_LCD_line(IN_X_SEC_MSG, LCD_LINE_3);
				for (uint8_t ptr = 5; ptr > 0; ptr--) {
					print_LCD_char(ptr + '0',LCD_LINE_4, 9); // X position
					_delay_ms(DELAY_COMMAND_MS);
				}
				play_melody(false);
				RESET_DEVICE();
				break;
					
			case BOOT_FROM_APP: 
				if (UART.rx_buffer[7] == 'D') { // Direct request.
					eeprom_write_byte((uint8_t *)STORED_CONNECTION_STATE_ADDRESS, 'D');
					clear_LCD();
					print_LCD_line(REBOOT_MSG, LCD_LINE_2);
					print_LCD_line(IN_X_SEC_MSG, LCD_LINE_3);
					for (uint8_t ptr = 5; ptr > 0; ptr--) {
						print_LCD_char(ptr + '0',LCD_LINE_3, 9); // X position
						_delay_ms(DELAY_COMMAND_MS);
					}	
					RESET_DEVICE();
				}
				else if (UART.rx_buffer[7] == 'L') { // LAN request
					eeprom_write_byte((uint8_t *)STORED_CONNECTION_STATE_ADDRESS, 'L');
					clear_LCD();
					print_LCD_line(REBOOT_MSG, LCD_LINE_2);
					print_LCD_line(IN_X_SEC_MSG, LCD_LINE_3);
					for (uint8_t ptr = 5; ptr > 0; ptr--) {
						print_LCD_char(ptr + '0',LCD_LINE_3, 9); // X position
						_delay_ms(DELAY_COMMAND_MS);
					}
					RESET_DEVICE();
				}
				else if (UART.rx_buffer[7] == 'S')  { // Shutdown
					clear_LCD();
					print_LCD_line(SHUTDOWN_MSG, LCD_LINE_2);
					print_LCD_line(IN_X_SEC_MSG, LCD_LINE_3);
					for (uint8_t ptr = 5; ptr > 0; ptr--) {
						print_LCD_char(ptr + '0',LCD_LINE_3, 9); // X position
						_delay_ms(DELAY_COMMAND_MS);
					}
					play_melody(false);
					DISABLE_DEVICE();
				}
				else if ((UART.rx_buffer[7] == 'R') && (UART.rx_buffer[10] == 'T'))  { // Shutdown
					clear_LCD();
					print_LCD_line("Reset to factory... ", LCD_LINE_2);
					print_LCD_line(IN_X_SEC_MSG, LCD_LINE_3);
					for (uint8_t ptr = 5; ptr > 0; ptr--) {
						print_LCD_char(ptr + '0',LCD_LINE_3, 9); // X position
						_delay_ms(DELAY_COMMAND_MS);
					}
					play_melody(false);
					erase_EEPROM_1K();
					RESET_DEVICE();
				}
				else if ((UART.rx_buffer[7] == 'R') && (UART.rx_buffer[10] == 'E'))  { // Shutdown
					clear_LCD();
					print_LCD_line(REBOOT_MSG, LCD_LINE_2);
					print_LCD_line(IN_X_SEC_MSG, LCD_LINE_3);
					for (uint8_t ptr = 5; ptr > 0; ptr--) {
						print_LCD_char(ptr + '0',LCD_LINE_3, 9); // X position
						_delay_ms(DELAY_COMMAND_MS);
					}
					play_melody(false);
					RESET_DEVICE();
				}	
				else msg_state = MESSAGE_HANDLER_IDLE;			
//				send_command_UART("OK\r\n");	
//				ENABLE_TIMER();			
				break;
				
			case LCD_PARAM_ADJUST:
				switch(UART.rx_buffer[6]) {
					case 'B': set_LCD_brightness(update_sg_param_value(BRIGHTNESS));  break;
					case 'C': set_LCD_contrast(update_sg_param_value(CONTRAST));  break;
					default: break;
				}
				clear_uart_rx_message();
				msg_state = MESSAGE_HANDLER_IDLE;
//				send_command_UART("OK\r\n");
//				ENABLE_TIMER();
				break;
				
			case VOLUME_ADJUST:
				set_volume(update_sg_param_value(VOLUME));
				clear_uart_rx_message();
				msg_state = MESSAGE_HANDLER_IDLE;
//				send_command_UART("OK\r\n");
//				ENABLE_TIMER();
			break;
				
		}
	}
}

void set_volume(uint8_t value) {
	value = (value * MAX_8BIT) / 100;
	SPI_write_16bit(VOLUME_ADDR, 255 - value,LCD_POT);	
}

uint8_t update_sg_param_value(uint8_t parameter) {
	uint8_t val = 0;
	if (UART.rx_buffer[7] == '\r') return 0;
	else if (UART.rx_buffer[8] == '\r') val = (UART.rx_buffer[7] - '0');
	else if (UART.rx_buffer[9] == '\r') val = (UART.rx_buffer[8] - '0') + ((UART.rx_buffer[7] - '0') * 10);
	else if (UART.rx_buffer[10] == '\r') val = (UART.rx_buffer[9] - '0') + ((UART.rx_buffer[8] - '0') * 10) + ((UART.rx_buffer[7] - '0') * 100);
	else return 100; // wrong value
	if (parameter == CONTRAST) LCD.contrast = val;
	else if (parameter == BRIGHTNESS) LCD.brightness = val;
	else if (parameter == VOLUME) LCD.volume = val;
	return val;
}

void update_battery_status() {
	uint16_t ta, tb = 0;
	ADMUX = 0;
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
	ta = ADC * BATTERY_ADC_FACTOR_A;
	tb = ta * BATTERY_ADC_FACTOR_B;
	STATUS.battery_voltage = get_li_ion_percentage(tb);
}

void update_ac_power_status() {
	volatile uint16_t adcx = 0;
	ADMUX |= 1;
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
	adcx = ADC;
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
	adcx = (adcx + ADC) / 2; // Double sum average
	if (adcx < POWER_ADC_THRESHOLD) STATUS.ac_power_status = false;
	else STATUS.ac_power_status = true;
}

char * get_battery_status() {
	int sprintf_store = 0;
	static char command[BATTERY_CMD_LENGTH];
	memset(command,0,BATTERY_CMD_LENGTH);
	sprintf_store = snprintf(command,11, "VBAT=%d\r\n",STATUS.battery_voltage);//status.battery_voltage);
	return command;
}

char * get_ac_power_status() {
	int sprintf_store = 0;
	static char command[POWER_CMD_LENGTH];
	memset(command,0,POWER_CMD_LENGTH);
	if (STATUS.ac_power_status) sprintf_store = snprintf(command, 7, "AC=ON\r\n"); // TRUE - ON
	else sprintf_store = snprintf(command, 9, "AC=OFF\r\n");
	return command;
}

uint8_t get_li_ion_percentage (uint16_t vin) {
	if (vin < 3300) return 0;
	else if ((vin >= 3300) && (vin < 3800)) return (uint8_t)((vin - 3300)*36*0.002);
	else if ((vin >= 3800) && (vin < 3900)) return (uint8_t)(36 + ((vin - 3800)*26*0.01));
	else if ((vin >= 3900) && (vin < 4000)) return (uint8_t)(62 + ((vin - 3900)*11*0.01));
	else if ((vin >= 4000) && (vin < 4100)) return (uint8_t)(73 + ((vin - 4000)*16*0.01));
	else if ((vin >= 4100) && (vin < 4200)) return (uint8_t)(89 + ((vin - 4100)*11*0.01));
	else return 100;
}					

void clear_active_UI() {
	memset(UI.frequency_A, 0, 8);
	memset(UI.frequency_B, 0, 8);
	memset(UI.amplitude_A, 0, 4);
	memset(UI.amplitude_B, 0, 4);
	memset(UI.bias_A, 0, 5);
	memset(UI.bias_B, 0, 5);
	memset(UI.type_A, 0, 4);
	memset(UI.type_B, 0, 4);
}


void update_UI_activity(uint8_t line, uint8_t segment) {
	int sprintf_store = 0;
	char bufX[MAX_STRING_BUFFER];
	clear_LCD_segment(line, segment);
	memset(bufX, 0, 20);
	switch(line) {
		case LCD_LINE_1: 
			if (segment == 0) sprintf_store = snprintf(bufX, 7, "A:%s|", UI.type_A);   
			else sprintf_store = snprintf(bufX, 13, "f:%s Hz", UI.frequency_A);
			break;
		case LCD_LINE_2: 
			if (segment == 0) sprintf_store = snprintf(bufX, 10, "VA=%sV|", UI.amplitude_A);
			else sprintf_store = snprintf(bufX, 11, "VbA=%sV", UI.bias_A); 
			break;
		case LCD_LINE_3: 
			if (segment == 0) sprintf_store = snprintf(bufX, 7, "B:%s|", UI.type_B);
			else sprintf_store = snprintf(bufX, 13, "f:%s Hz", UI.frequency_B); 
			break;
		case LCD_LINE_4:
			if (segment == 0) sprintf_store = snprintf(bufX, 10, "VB=%sV|", UI.amplitude_B); 
			else sprintf_store = snprintf(bufX, 11, "VbB=%sV", UI.bias_B); 
			break;
		default: break;		
	}
	print_LCD_segment(bufX, line, segment);
}

void update_complete_UI() {
	update_UI_activity(LCD_LINE_1, 0);
	update_UI_activity(LCD_LINE_1, 1);
	update_UI_activity(LCD_LINE_2, 0);
	update_UI_activity(LCD_LINE_2, 1);
	update_UI_activity(LCD_LINE_3, 0);
	update_UI_activity(LCD_LINE_3, 1);
	update_UI_activity(LCD_LINE_4, 0);
	update_UI_activity(LCD_LINE_4, 1);

}

uint32_t retrieve_frequency_uint32(char channel_in) {
	volatile uint8_t mark_ptr = 0;
	volatile uint8_t end_of_freq_string = 0;
	volatile uint32_t return_number = 0;
	if (channel_in == PARAM_CH0) memset(UI.frequency_A, 0, 8);
	else memset(UI.frequency_B, 0, 8);
	for (mark_ptr = FG_DATA_START_NUM; mark_ptr < FG_DATA_LENGTH; mark_ptr++) if (UART.rx_buffer[mark_ptr + 1] == '\"') { end_of_freq_string = mark_ptr; break; }
	for (mark_ptr = end_of_freq_string; mark_ptr >= FG_DATA_START_NUM; mark_ptr--) {
		return_number += (UART.rx_buffer[mark_ptr] - 0x30) * pow(10, end_of_freq_string - mark_ptr);
		if (channel_in == PARAM_CH0) UI.frequency_A[mark_ptr - FG_DATA_START_NUM] = UART.rx_buffer[mark_ptr];
		else UI.frequency_B[mark_ptr - FG_DATA_START_NUM] = UART.rx_buffer[mark_ptr];
	}
	return return_number + 1;
}

uint16_t retrieve_amplitude_12_bit(char channel_in) { // MAX 700mV
	int sprintf_store = 0;
	volatile uint32_t num = 0;
	char buffer[3];
	buffer[0] = UART.rx_buffer[9];
	buffer[1] = UART.rx_buffer[11];
	buffer[2] = UART.rx_buffer[12];
	num = atoi(buffer);
	if (channel_in == PARAM_CH0) memset(UI.amplitude_A, 0, 8);
	else memset(UI.amplitude_B, 0, 8);
	if (channel_in == PARAM_CH0) { sprintf_store = snprintf(UI.amplitude_A, 5, "%c.%c%c",buffer[0],buffer[1],buffer[2]); }
	else sprintf_store = snprintf(UI.amplitude_B, 5, "%c.%c%c",buffer[0],buffer[1],buffer[2]);
	return ((num * MAX_12BIT) / 70);
}

uint16_t retrieve_bias_12_bit(char channel_in) {
	int sprintf_store = 0;
	volatile uint32_t num = 0;
	char buffer[3];
	buffer[0] = UART.rx_buffer[10];
	buffer[1] = UART.rx_buffer[12];
	buffer[2] = UART.rx_buffer[13];
	num = atoi(buffer);
	if (channel_in == PARAM_CH0) memset(UI.bias_A, 0, 8);
	else memset(UI.bias_B, 0, 8);
	if (channel_in == PARAM_CH0) sprintf_store = snprintf(UI.bias_A, 6, "%c%c.%c%c",UART.rx_buffer[9],buffer[0],buffer[1],buffer[2]);
	else sprintf_store = snprintf(UI.bias_B, 6, "%c%c.%c%c",UART.rx_buffer[9],buffer[0],buffer[1],buffer[2]);
	return ((num * MAX_12BIT) / 330);
	}

bool retrieve_bias_sign() {
	if (UART.rx_buffer[9] == '-') return NEGATIVE;
	else if (UART.rx_buffer[9] == '+') return POSITIVE;
	else return NULL;
}

void set_parameter() {
	if (UART.rx_buffer[4] == PARAM_CH0) {
		switch(UART.rx_buffer[6]) {
			case PARAM_FREQ: 
				FUNCGEN.frequency_A = retrieve_frequency_uint32(PARAM_CH0);
				set_functionality(FG0, FUNCGEN.frequency_A, FUNCGEN.output_type_A);
				update_UI_activity(LCD_LINE_1, 1);
				break;
				
			case PARAM_TYPE:
				switch(UART.rx_buffer[9]) {
					case TYPE_SINE: FUNCGEN.output_type_A = SINE; snprintf(UI.type_A, 4, "%s","SIN"); break;
					case TYPE_SQUARE: FUNCGEN.output_type_A = SQUARE; snprintf(UI.type_A, 4, "%s","SQR"); break;
					case TYPE_TRIANGLE: FUNCGEN.output_type_A = TRIANGLE; snprintf(UI.type_A, 4, "%s","TRG"); break;
					case TYPE_DC	  : FUNCGEN.output_type_A = DC; snprintf(UI.type_A, 4, "%s","DC "); break;
					case TYPE_OFF	  : FUNCGEN.output_type_A = OFF; snprintf(UI.type_A, 4, "%s","OFF"); break;
					default: break;
				}
				set_functionality(FG0, FUNCGEN.frequency_A, FUNCGEN.output_type_A);
				update_UI_activity(LCD_LINE_1, 0);
				break;
				
			case PARAM_AMP:
				FUNCGEN.amplitude_A = retrieve_amplitude_12_bit(PARAM_CH0);
				set_amplitude(FUNCGEN.amplitude_A, FG0);
				update_UI_activity(LCD_LINE_2, 0);
				break;
				
			case PARAM_BIAS: 
				FUNCGEN.bias_A = retrieve_bias_12_bit(PARAM_CH0);
				FUNCGEN.bias_A_sign = retrieve_bias_sign(); 
				if (FUNCGEN.bias_A_sign == POSITIVE) set_dc_bias(FG0, 0, NEGATIVE);
				else set_dc_bias(FG0, 0, POSITIVE);
				set_dc_bias(FG0, FUNCGEN.bias_A, FUNCGEN.bias_A_sign);
				update_UI_activity(LCD_LINE_2, 1);
				break;
				
			default: break;
		}
		
	}
	else if (UART.rx_buffer[4] == PARAM_CH1) { 
		switch(UART.rx_buffer[6]) {
			case PARAM_FREQ: 
				FUNCGEN.frequency_B = retrieve_frequency_uint32(PARAM_CH1);
				set_functionality(FG1, FUNCGEN.frequency_B, FUNCGEN.output_type_B);
				update_UI_activity(LCD_LINE_3, 1);
				break;
				
			case PARAM_TYPE:
				switch(UART.rx_buffer[9]) {
					case TYPE_SINE: FUNCGEN.output_type_B = SINE; snprintf(UI.type_B, 4, "%s","SIN"); break;
					case TYPE_SQUARE: FUNCGEN.output_type_B = SQUARE;snprintf(UI.type_B, 4, "%s","SQR"); break;
					case TYPE_TRIANGLE: FUNCGEN.output_type_B = TRIANGLE; snprintf(UI.type_B, 4, "%s","TRG"); break;
					case TYPE_DC	  : FUNCGEN.output_type_B = DC; snprintf(UI.type_B, 4, "%s","DC "); break;
					case TYPE_OFF	  : FUNCGEN.output_type_B = OFF;snprintf(UI.type_B, 4, "%s","OFF"); break;
					default: break;
				}
				set_functionality(FG1, FUNCGEN.frequency_B, FUNCGEN.output_type_B);
				update_UI_activity(LCD_LINE_3, 0);
				break;
				
			case PARAM_AMP:
				FUNCGEN.amplitude_B = retrieve_amplitude_12_bit(PARAM_CH1);
				set_amplitude(FUNCGEN.amplitude_B, FG1);
				update_UI_activity(LCD_LINE_4, 0);
				break;
				
			case PARAM_BIAS: 
				FUNCGEN.bias_B = retrieve_bias_12_bit(PARAM_CH1);
				FUNCGEN.bias_B_sign = retrieve_bias_sign(); 
				if (FUNCGEN.bias_B_sign == POSITIVE) set_dc_bias(FG1, 0, NEGATIVE);
				else set_dc_bias(FG1, 0, POSITIVE);
				set_dc_bias(FG1, FUNCGEN.bias_B, FUNCGEN.bias_B_sign);
				update_UI_activity(LCD_LINE_4, 1);
				break;
				
			default: break;
		}
	}
}
	
void clear_all_values() {
	clear_funcgen_values();
	clear_wifi_values();
	clear_uart_rx_message();
	update_ac_power_status();
	update_battery_status();
}

void clear_funcgen_values() {
	FUNCGEN.frequency_A = 0; 
	FUNCGEN.frequency_B = 0;
	FUNCGEN.phase_A = 0; 
	FUNCGEN.phase_B = 0;
	FUNCGEN.amplitude_A = 0; 
	FUNCGEN.amplitude_B = 0;
	FUNCGEN.output_type_A = OFF; 
	FUNCGEN.output_type_B = OFF;
	FUNCGEN.bias_A = 0; 
	FUNCGEN.bias_B = 0;
}

void init_UI_array() {
	snprintf(UI.amplitude_A, 4, ZERO_AMPLITUDE);
	snprintf(UI.amplitude_B, 4, ZERO_AMPLITUDE);
	snprintf(UI.bias_A, 5, ZERO_BIAS);
	snprintf(UI.bias_B, 5, ZERO_BIAS);
	snprintf(UI.frequency_A, 8,"0");
	snprintf(UI.frequency_B, 8,"0");
	_delay_ms(10);
}

void clear_wifi_values() {
	memset(WIFI.SSID, 0, MAX_WIFI_SSID_LENGTH);
	memset(WIFI.PASS, 0, MAX_WIFI_PASS_LENGTH);
	memset(WIFI.device_MAC, 0, MAC_STRING_LENGTH);
	WIFI.encryption = 0;
}

int main() {
	/* Testing definitions */
	#ifdef PRE_PROG
		ENABLE_DEVICE();
	#else
		enum MENU_STATES_MAIN main_menu_state = INIT_STATE;
		Init_Device();
		Init_UI();
		sei();

	#ifdef TEST_BUZZER
		while(1){
			beep();
			_delay_ms(1000);
			play_melody(true);
			_delay_ms(1000);
			play_melody(false);
			_delay_ms(1000);
		}
	#endif
	
	#ifdef TEST_ADC 
		clear_LCD();
		while(1) {
			static char buff_line[20];
			ADMUX = 0;
			ADCSRA |= (1 << ADSC);
			while(ADCSRA & (1 << ADSC));
			adscx1 = ADC;
			sprintf(buff_line, "VBAT ADC Val:%d",ADC);
			print_LCD_line(buff_line,LCD_LINE_2);
			_delay_ms(500);
			ADMUX |= 1;
			ADCSRA |= (1 << ADSC);
			while(ADCSRA & (1 << ADSC));
			adscx2 = ADC;
			sprintf(buff_line, "AC ADC Val:%d",ADC);
			print_LCD_line(buff_line,LCD_LINE_3);
			_delay_ms(500);
		}
	#endif
	
	#ifdef TEST_UART 
// 	enum UART_RX_MESSAGE_TYPES { WAIT_FOR_OK = 1, WAIT_FOR_GOT_IP, WAIT_FOR_READY_TO_SEND,
// 		WAIT_FOR_CONNECT, WAIT_FOR_CLOSED, WAIT_FOR_SEND_OK,
// 	WAIT_FOR_READY, WAIT_FOR_DATA ,WAIT_FOR_CREDENTIALS ,WAIT_FOR_MAC };
		sei();
		while(1) {
		send_command_UART("Device is ready...\r\n");
		send_command_UART("UART.wait_for_message = WAIT_FOR_OK\r\n");
		UART.wait_for_message = WAIT_FOR_OK;
		while(!UART.message_received);
		send_command_UART("Valid message: ");
		send_command_UART(UART.rx_buffer);
		clear_uart_rx_message();
		
		UART.wait_for_message = WAIT_FOR_GOT_IP;
		while(!UART.message_received);
		send_command_UART("Valid message: \r\n");
		send_command_UART("UART.wait_for_message = WAIT_FOR_READY_TO_SEND\r\n");
		send_command_UART(UART.rx_buffer);
		clear_uart_rx_message();
		
		UART.wait_for_message = WAIT_FOR_READY_TO_SEND;
		while(!UART.message_received);
		send_command_UART("Valid message: \r\n");
		send_command_UART("UART.wait_for_message = WAIT_FOR_CONNECT\r\n");
		send_command_UART(UART.rx_buffer);
		clear_uart_rx_message();
		
		UART.wait_for_message = WAIT_FOR_CONNECT;
		while(!UART.message_received);
		send_command_UART("Valid message: \r\n");
		send_command_UART("UART.wait_for_message = WAIT_FOR_CLOSED\r\n");
		send_command_UART(UART.rx_buffer);
		clear_uart_rx_message();
		
		UART.wait_for_message = WAIT_FOR_CLOSED;
		while(!UART.message_received);
		send_command_UART("Valid message: \r\n");
		send_command_UART("UART.wait_for_message = WAIT_FOR_SEND_OK\r\n");
		send_command_UART(UART.rx_buffer);
		clear_uart_rx_message();
		
		UART.wait_for_message = WAIT_FOR_SEND_OK;
		while(!UART.message_received);
		send_command_UART("Valid message: \r\n");
		send_command_UART("UART.wait_for_message = WAIT_FOR_READY\r\n");
		send_command_UART(UART.rx_buffer);
		clear_uart_rx_message();
		
		UART.wait_for_message = WAIT_FOR_READY;
		while(!UART.message_received);
		send_command_UART("Valid message: \r\n");
		send_command_UART("UART.wait_for_message = WAIT_FOR_DATA\r\n");
		send_command_UART(UART.rx_buffer);
		clear_uart_rx_message();
		
		UART.wait_for_message = WAIT_FOR_DATA;
		while(!UART.message_received);
		send_command_UART("Valid message: \r\n");
		send_command_UART("UART.wait_for_message = WAIT_FOR_CREDENTIALS\r\n");
		send_command_UART(UART.rx_buffer);
		clear_uart_rx_message();
		
		UART.wait_for_message = WAIT_FOR_CREDENTIALS;
		while(!UART.message_received);
		send_command_UART("Valid message: \r\n");
		send_command_UART("UART.wait_for_message = WAIT_FOR_OK\r\n");
		send_command_UART(UART.rx_buffer);
		clear_uart_rx_message();
		
		UART.wait_for_message = WAIT_FOR_MAC;
		while(!UART.message_received);
		send_command_UART("Valid message: \r\n");
		send_command_UART("UART.wait_for_message = WAIT_FOR_MAC\r\n");
		send_command_UART(UART.rx_buffer);
		clear_uart_rx_message();
				
		}
		#endif
		
	while(1) {
		switch(main_menu_state) {
			
			case INIT_STATE: // Check EEPROM saved state
				switch (eeprom_read_byte(STORED_CONNECTION_STATE_ADDRESS)) {
					
					case WIFI_LAN_CONNECTON: 
						start_wlan_communication(); 
						break;
					case WIFI_DIRECT_CONNECTION:
						clear_LCD();
						print_LCD_line(START_DIRECT_MSG, LCD_LINE_1);
						print_LCD_line(START_COMM_MSG, LCD_LINE_2);
						print_LCD_line(PB_MSG, LCD_LINE_4);
						while(!poll_switch());
						main_menu_state = DIRECT_PAIRING_INIT;
						sei();
						break;
					
					case NO_STORED_CONNECTION: default:
						clear_LCD();
						print_LCD_line("  No Defined Wi-Fi  ", LCD_LINE_2);
						print_LCD_line("Connection Available", LCD_LINE_3);
						_delay_ms(3000);
						main_menu_state = PROCEED_BUTTON_PERFORM;
						break;
				}
				break;
			case PROCEED_BUTTON_PERFORM:
				clear_LCD();
				print_LCD_line(PB_MSG, LCD_LINE_2);
				while(!poll_switch());
				main_menu_state = BUTTON_STATE_SELECTION;
				break;
			
			case BUTTON_STATE_SELECTION:
				clear_LCD();
				print_LCD_line(START_DIRECT_MSG, LCD_LINE_1);
				print_LCD_line(START_COMM_MSG, LCD_LINE_2);
				print_LCD_line(PB_MSG, LCD_LINE_3);
				print_LCD_line("WLAN Pairing.. X sec", LCD_LINE_4);
				//sei();
				for (uint8_t ptr = 5; ptr > 0; ptr--) {
					print_LCD_char(ptr + '0',LCD_LINE_4, 15); // X position
					_delay_ms(1000);
					if (poll_switch()) {
						main_menu_state = WIFI_LAN_PAIRING_INIT;
						break;
					}
				}
				if (main_menu_state != WIFI_LAN_PAIRING_INIT) {
					sei();
					main_menu_state = DIRECT_PAIRING_INIT;
				}
				break;
				
			case DIRECT_PAIRING_INIT:
				main_menu_state = IDLE;
				sei();
				direct_pairing();
				break;
			
			case WIFI_LAN_PAIRING_INIT:
				main_menu_state = IDLE;
				sei();
				wifi_lan_pairing();
				break;
				
			case IDLE: break;
				
		}
	}
	#endif			
}