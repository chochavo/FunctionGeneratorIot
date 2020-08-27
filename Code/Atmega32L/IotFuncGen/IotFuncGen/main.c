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
	ENCODER_A_DDR &= ~ENCODER_A;
	ENCODER_B_DDR &= ~ENCODER_B;
	ENCODER_A_PORT |= ENCODER_A;
	ENCODER_B_PORT |= ENCODER_B;
	SPI_CE_DDR |= DACA_NCE | DACB_NCE | DACA_BIAS_NCE | DACB_BIAS_NCE | FG0_NCE | FG1_NCE;
	LCD_POT_DDR |= POT_LCD_NCE;
	LED_DDR |= LED_R | LED_G | LED_B;
	LCD_CONTROL_DDR |= LCD_RS | LCD_E;
	LCD_DATA_DDR |= LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7;
	ADC_DDR &= ~VBAT_ADC & ~PWR_IND;
	SPI_DDR |= SPI_MOSI | SPI_SCK;
	SPI_DDR &= ~SPI_MISO;
	UART_DDR |= UART_TX;
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

int set_dc_bias(enum DEVICES device, uint16_t value, bool sign) {
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

uint8_t update_sg_param_value(uint8_t parameter) {
	uint8_t val = 0;
	if (parameter == CONTRAST) LCD.contrast = val;
	else if (parameter == BRIGHTNESS) LCD.brightness = val;
	return val;
}

void update_battery_status() {
	uint16_t ta, tb = 0;
	ADMUX = 0;
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
	ta = ADC * BATTERY_ADC_FACTOR_A;
	STATUS.battery_voltage = ta * BATTERY_ADC_FACTOR_B;
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

void clear_funcgen_values() {
	FUNCGEN.frequency_A = 0; 
	FUNCGEN.frequency_B = 0;
	FUNCGEN.amplitude_A = 0; 
	FUNCGEN.amplitude_B = 0;
	FUNCGEN.output_type_A = OFF; 
	FUNCGEN.output_type_B = OFF;
	FUNCGEN.bias_A = 0; 
	FUNCGEN.bias_B = 0;
	FUNCGEN.bias_A_sign = POSITIVE;
	FUNCGEN.bias_B_sign = POSITIVE;
}

int main() {
	/* Testing definitions */
	#ifdef PRE_PROG
		ENABLE_DEVICE();
		while(1);
	#endif
	
	Screen display;
	UI displayParameters;
	uint8_t displayPointer;
	volatile EncoderState encoderState = NONE;
	bool switchState = false;
	bool pointerIsActive = false;
	bool switchPressed = true;
	
	Init_Device();
	Init_UI();
	sei();
	
	clear_LCD();
	print_LCD_line("	 Loading...     ", LCD_LINE_2);
	
	/* Initialization sequence */
	display.currentScreen = CH_A;
	display.mainScreen = MAIN_SCREEN;
	display.stateChanged = false;
	
	sprintf(displayParameters.frequency_A, "%d", FUNCGEN.frequency_A);
	sprintf(displayParameters.frequency_B, "%d", FUNCGEN.frequency_B);
	sprintf(displayParameters.bias_A, "%d", FUNCGEN.bias_A);
	sprintf(displayParameters.bias_B, "%d", FUNCGEN.bias_B);
	sprintf(displayParameters.type_A, "%s", MODES_STRING[FUNCGEN.output_type_A]);
	sprintf(displayParameters.type_B, "%s", MODES_STRING[FUNCGEN.output_type_B]);
	sprintf(displayParameters.amplitude_A, "%d", FUNCGEN.amplitude_A);
	sprintf(displayParameters.amplitude_B, "%d", FUNCGEN.amplitude_B);
	
	if (FUNCGEN.bias_A_sign == POSITIVE) displayParameters.bias_A_sign = '+';
	else displayParameters.bias_A_sign = '-';
	if (FUNCGEN.bias_B_sign == POSITIVE) displayParameters.bias_B_sign = '+';
	else displayParameters.bias_B_sign = '-';
	_delay_ms(1500);	
	print_LCD_screen(display.mainScreen, display.currentScreen, displayParameters);
	
	while(1) {
		if (display.stateChanged) {
			display.stateChanged = false;
		}
			
		encoderState = poll_encoder();
		switchState = poll_switch();
		
		if (switchState) {
			pointerIsActive != pointerIsActive;
			switchPressed = true;
		}
		
		if (!pointerIsActive) {
			switch(display.mainScreen) {
				case MAIN_SCREEN:
					if (encoderState == CW) {
						if (display.currentScreen == CH_A) {
							display.currentScreen == CH_B;
							display.stateChanged = true;
						}
						else if (display.currentScreen == CH_B) {
							display.currentScreen == SETT;
							display.mainScreen = PARAMS_SCREEN;
							display.stateChanged = true;
						}
					}
					else if (encoderState == CCW) {
						if (display.currentScreen == CH_B) {
							display.currentScreen == CH_A;
							display.stateChanged = true;
						}
						else if (display.currentScreen == SETT) {
							display.currentScreen == CH_B;
							display.mainScreen = MAIN_SCREEN;
							display.stateChanged = true;
						}
					}
			}
		}
	}
}

#define TYPE_ADDR 0x00 // 'S', 'T', 'Q', 'D', 'O'
#define FREQ_ADDR 0x01 // 32-bit: 0x01, 0x02, 0x03, 0x04 LSB
#define BIAS_SIGN_ADDR 0x05 // '+'
#define BIAS_VALUE_ADDR 0x06 // 2 bytes - 0x06, 0x07 LSB
#define AMPLITUDE_ADDR 0x08 // 2 bytes - 0x08, 0x09 LSB
#define CHANNEL_B_ADDR 0x10

typedef struct UI_STRINGS {
	char frequency_A[7], frequency_B[7];
	char amplitude_A[3], amplitude_B[3];
	char bias_A[3], bias_B[3];
	char type_A[3], type_B[3];
	char bias_A_sign, bias_B_sign;
	char ac_status, battery_status;
} UI;


void print_LCD_screen(MainScreen mainScreen, CurrentScreen currentScreen, UI *displayParams) {
	char lcdBuffer[4][20];
	switch (mainScreen) {
		case MAIN_SCREEN:
			memcpy(lcdBuffer[0], LCD_MAIN_STRING_1, 20);
			memcpy(lcdBuffer[1], LCD_MAIN_STRING_2, 20);
			memcpy(lcdBuffer[2], LCD_MAIN_STRING_3, 20);
			memcpy(lcdBuffer[3], LCD_MAIN_STRING_4, 20);
			if (currentScreen == CH_A) {
				memcpy(lcdBuffer[0] + 17, displayParams->type_A);
				lcdBuffer[9] = 'A';
				lcdBuffer[1][12] = displayParams->amplitude_A[0];
				lcdBuffer[1][14] = displayParams->amplitude_A[1];
				lcdBuffer[1][15] = displayParams->amplitude_A[2];
				
				lcdBuffer[2][7] = displayParams->frequency_A[0];
				lcdBuffer[2][9] = displayParams->frequency_A[1];
				lcdBuffer[2][10] = displayParams->frequency_A[2];
				lcdBuffer[2][11] = displayParams->frequency_A[3];
				lcdBuffer[2][13] = displayParams->frequency_A[4];
				lcdBuffer[2][14] = displayParams->frequency_A[5];
				lcdBuffer[2][15] = displayParams->frequency_A[6];
				
				lcdBuffer[3][7] = displayParams->bias_A_sign;	
							
				lcdBuffer[3][8] = displayParams->bias_A[0];
				lcdBuffer[3][10] = displayParams->bias_A[1];
				lcdBuffer[3][11] = displayParams->bias_A[2];				
			}
			else {
				memcpy(lcdBuffer[0] + 17, displayParams->type_B);
				lcdBuffer[9] = 'B';
				lcdBuffer[1][12] = displayParams->amplitude_B[0];
				lcdBuffer[1][14] = displayParams->amplitude_B[1];
				lcdBuffer[1][15] = displayParams->amplitude_B[2];
			
				lcdBuffer[2][7] = displayParams->frequency_B[0];
				lcdBuffer[2][9] = displayParams->frequency_B[1];
				lcdBuffer[2][10] = displayParams->frequency_B[2];
				lcdBuffer[2][11] = displayParams->frequency_B[3];
				lcdBuffer[2][13] = displayParams->frequency_B[4];
				lcdBuffer[2][14] = displayParams->frequency_B[5];
				lcdBuffer[2][15] = displayParams->frequency_B[6];
			
				lcdBuffer[3][7] = displayParams->bias_B_sign;
			
				lcdBuffer[3][8] = displayParams->bias_B[0];
				lcdBuffer[3][10] = displayParams->bias_B[1];
				lcdBuffer[3][11] = displayParams->bias_B[2];
			}
			break;
			
		case PARAMS_SCREEN:
			memcpy(lcdBuffer[0], LCD_MAIN_SETTINGS_STRING_1, 20);
			memcpy(lcdBuffer[1], LCD_MAIN_SETTINGS_STRING_2, 20);
			memcpy(lcdBuffer[2], LCD_MAIN_SETTINGS_STRING_3, 20);
			memcpy(lcdBuffer[3], LCD_MAIN_SETTINGS_STRING_4, 20);
			
			lcdBuffer[0][13] = displayParams->battery_status[0];
			lcdBuffer[0][15] = displayParams->battery_status[1];
			lcdBuffer[0][16] = displayParams->battery_status[2];
			
			memcpy(lcdBuffer[1], displayParams->ac_status, 3);
						
			break;
			
		case SETTINGS_SCREEN:
			memcpy(lcdBuffer[0], LCD_SETTINGS_STRING_1, 20);
			memcpy(lcdBuffer[1], LCD_SETTINGS_STRING_2, 20);
			memcpy(lcdBuffer[2], LCD_SETTINGS_STRING_3, 20);
			memcpy(lcdBuffer[3], LCD_SETTINGS_STRING_4, 20);
			break;
			
		case PROFILE_SCREEN:
			memcpy(lcdBuffer[0], LCD_PROFILE_SETTINGS_STRING_1, 20);
			memcpy(lcdBuffer[1], LCD_PROFILE_SETTINGS_STRING_2, 20);
			memcpy(lcdBuffer[2], LCD_PROFILE_SETTINGS_STRING_3, 20);
			memcpy(lcdBuffer[3], LCD_PROFILE_SETTINGS_STRING_4, 20);
			break;
			
		case LCD_SCREEN:
			memcpy(lcdBuffer[0], LCD_SCREEN_SETTINGS_STRING_1, 20);
			memcpy(lcdBuffer[1], LCD_SCREEN_SETTINGS_STRING_2, 20);
			memcpy(lcdBuffer[2], LCD_SCREEN_SETTINGS_STRING_3, 20);
			memcpy(lcdBuffer[3], LCD_SCREEN_SETTINGS_STRING_4, 20);
			break;
			
		case SHUTDOWN_SCREEN:
			memcpy(lcdBuffer[0], LCD_SHUTDOWN_STRING_1, 20);
			memcpy(lcdBuffer[1], LCD_SHUTDOWN_STRING_2, 20);
			memcpy(lcdBuffer[2], LCD_SHUTDOWN_STRING_3, 20);
			memcpy(lcdBuffer[3], LCD_SHUTDOWN_STRING_4, 20);
			break;
			
		default: break;
	}
	print_LCD_line(lcdBuffer[0], LCD_LINE_1);
	print_LCD_line(lcdBuffer[1], LCD_LINE_2);
	print_LCD_line(lcdBuffer[2], LCD_LINE_3);
	print_LCD_line(lcdBuffer[3], LCD_LINE_4);
}

#define LCD_MAIN_STRING_1 " Channel:X, Type:XXX"
#define LCD_MAIN_STRING_2 " Amplitude: X.XX[V] "
#define LCD_MAIN_STRING_3 " Freq: X.XXX.XXX[Hz]"
#define LCD_MAIN_STRING_4 " Bias: XX.XX[V]     "

#define LCD_MAIN_SETTINGS_STRING_1 "Voltage(Bat):X.XX[V]"
#define LCD_MAIN_SETTINGS_STRING_2 "External Power: XXX "
#define LCD_MAIN_SETTINGS_STRING_3 "  Settings		   "
#define LCD_MAIN_SETTINGS_STRING_4 "  Shutdown	       "

#define LCD_SETTINGS_STRING_1 "  Profile Settings  "
#define LCD_SETTINGS_STRING_2 "  LCD Settings      "
#define LCD_SETTINGS_STRING_3 "  Boot Settings     "
#define LCD_SETTINGS_STRING_4 "       <BACK>       "

#define LCD_PROFILE_SETTINGS_STRING_1 "  Save Profile	   "
#define LCD_PROFILE_SETTINGS_STRING_2 "  Load Profile	   "
#define LCD_PROFILE_SETTINGS_STRING_3 "  Delete Profile	   "
#define LCD_PROFILE_SETTINGS_STRING_4 "       <BACK>       "

#define LCD_SCREEN_SETTINGS_STRING_1 "  Brightness:XXX[%] "
#define LCD_SCREEN_SETTINGS_STRING_2 "  Contrast:XXX[%]	  "
#define LCD_SCREEN_SETTINGS_STRING_3 "                    "
#define LCD_SCREEN_SETTINGS_STRING_4 "       <BACK>       "

#define LCD_SHUTDOWN_STRING_1 "  Perform Reset     "
#define LCD_SHUTDOWN_STRING_2 "  Power Off         "
#define LCD_SHUTDOWN_STRING_3 "  Factory Settings  "
#define LCD_SHUTDOWN_STRING_4 "       <BACK>       "

void LCD_logo_display() {
	char lbuff[20];
	print_LCD_line("Mobile Function Generator", LCD_LINE_1);
	snprintf(lbuff, 20, " Firmware V%c.%c.%c ", FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR, FIRMWARE_VERSION_BUILD);
	print_LCD_line(lbuff, LCD_LINE_2);
	print_LCD_line("  KhomLabs Design   ", LCD_LINE_3);
	print_LCD_line("  <Initializing...> ", LCD_LINE_4);
}

#define PIN_ENCODER
#define PORT_ENCODER
#define DDR_ENCODER
#define ENCODER_A
#define ENCODER_B
#define UI_PTR '>'
typedef enum EncoderStates {NONE, CW, CCW} EncoderState;
typedef enum MainScreens { MAIN_SCREEN, PARAMS_SCREEN, SETTINGS_SCREEN, PROFILE_SCREEN, LCD_SCREEN, SHUTDOWN_SCREEN } MainScreen;
typedef enum CurrentScreens { CH_A, CH_B, SETT } CurrentScreen;

typedef struct {
	bool stateChanged;
	CurrentScreen currentScreen;
	MainScreen mainScreen;
	} Screen;
	
typedef struct {
	volatile bool previousA;
	volatile bool previousB;
	uint8_t encoderSeqCntCW;
	uint8_t encoderSeqCntCCW;
	} Encoder;
	
EncoderState poll_encoder() {
	EncoderState state = NONE;
	volatile bool currentB = PIN_ENCODER & (1 << ENCODER_B);
	volatile bool currentA = PIN_ENCODER & (1 << ENCODER_A);
	/* State change */
	if ((Encoder.previousA && Encoder.previousB) && (!currentA && currentB)) Encoder.encoderSeqCntCW++;
	else if ((!Encoder.previousA && Encoder.previousB) && (!currentA && !currentB)) Encoder.encoderSeqCntCW++;
	else if ((!Encoder.previousA && !Encoder.previousB) && (currentA && !currentB)) Encoder.encoderSeqCntCW++;
	else if ((Encoder.previousA && !Encoder.previousB) && (currentA && currentB)) Encoder.encoderSeqCntCW++;
		
	else if ((Encoder.previousA && Encoder.previousB) && (currentA && !currentB)) Encoder.encoderSeqCntCCW++;
	else if ((Encoder.previousA && !Encoder.previousB) && (!currentA && !currentB)) Encoder.encoderSeqCntCCW++;
	else if ((!Encoder.previousA && !Encoder.previousB) && (!currentA && currentB)) Encoder.encoderSeqCntCCW++;
	else if ((!Encoder.previousA && Encoder.previousB) && (currentA && currentB)) Encoder.encoderSeqCntCCW++;
		
	if (Encoder.encoderSeqCntCW == 4) {
		Encoder.encoderSeqCntCW = 0;
		state = CW;
	}
	else if (Encoder.encoderSeqCntCCW == 4) {
		Encoder.encoderSeqCntCCW = 0;
		state = CCW;
	}
	else state = NONE;
		
	Encoder.previousA = currentA;
	Encoder.previousB = currentB;
	_delay_us(100);
	return state;
}