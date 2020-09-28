/*
 * IoT Dual Channel Function Generator V2.0
 * Designed by: Michael Khomyakov
 * July 2019
 */ 
//#include "main.h"
#include "Headers\main.h"
#include "Headers\SPIMaster.h"
#include "Headers\USART.h"
#include "Headers\LCD.h"
#include "Headers\Version.h"

 
void Buzzer_Beep() { // d - f - a
		for (uint8_t ptrm = 0; ptrm < 20; ptrm++) {
			MISC_PORT |= BUZZER;
			_delay_us(426);
			MISC_PORT &= ~BUZZER;
			_delay_us(426);
		}
}

void Buzzer_PlayMelody(bool power_on) {
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
		for (uint16_t ptrm = 0; ptrm < 652; ptrm++) {
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

void Main_ShutdownDevice(bool is_erase_requested) {
	Buzzer_Beep();
	LCD_Clear();
	LCD_PrintLine("<<Shutdown request>>", LCD_LINE_1);
	LCD_PrintLine("Device shutting down", LCD_LINE_2);
	LCD_PrintLine("in X sec            ", LCD_LINE_3);
	for (uint8_t cntx = 5; cntx > 0; cntx--) {
		LCD_PrintChar(cntx + '0',LCD_LINE_3, 3);
		_delay_ms(DELAY_COMMAND_MS);
	}
	Buzzer_PlayMelody(false);
	DISABLE_DEVICE();
}

void Init_Timer() {
	TIMSK |= (1 << OCIE1A);
	TCNT1 = 0;
	TCCR1B |= (1 << WGM12);
	OCR1A = 1000;
}

void Init_Ports() {
	ENCODER_DDR &= ~ENCODER_A & ~ENCODER_B;
	ENCODER_PORT |= ENCODER_A | ENCODER_B | (1 << 4);
	SPI_CE_DDR |= DACA_NCE | DACB_NCE | DACA_BIAS_NCE | DACB_BIAS_NCE | FG0_NCE | FG1_NCE;
	LCD_POT_DDR |= POT_LCD_NCE;
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
	Init_UART();
	ENABLE_DEVICE();
	SPI_InitAll();
	LCD_Init();
	_delay_ms(DELAY_COMMAND_MS);
	LCD_Init4bit();
	Init_ADC();
}

void Init_ADC() {
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);
	ADMUX |= (1 << REFS0);
}

void Init_UI() {
	LCD_BrightnessAnimation();
	LCD_SetBrightness(100);
	LCD_SetContrast(100);
	LCD_LogoDisplay();
	Buzzer_PlayMelody(true);
}

void FG_SelectOutputType(enum Device device, enum WaveformType waveformType) {
	switch(waveformType) {
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

void FG_SetFunction(enum Device device, uint32_t waveFrequency, enum WaveformType mode) {
	waveFrequency *= AD9834_FREQ_FACTOR;
	uint16_t freq_reg_lsb = waveFrequency & 0x0003FFF;
	uint16_t freq_reg_msb = ((waveFrequency >> 14) & 0x0003FFF);
	uint8_t freq_reg_lsb_a = ((freq_reg_lsb >> 8) & 0x3F);
	uint8_t freq_reg_lsb_b = (freq_reg_lsb & 0xFF);
	uint8_t freq_reg_msb_a = ((freq_reg_msb >> 8) & 0x3F);
	uint8_t freq_reg_msb_b = (freq_reg_msb & 0xFF);
	SPI_InitAD9834();
	SPI_Write16Bit(AD9834_CONSECUTIVE_WRITE,0x00,device);
	SPI_Write16Bit(AD9834_FREQUENCY_REGISTER_ADDR |freq_reg_lsb_a, freq_reg_lsb_b, device);
	SPI_Write16Bit(AD9834_FREQUENCY_REGISTER_ADDR |freq_reg_msb_a, freq_reg_msb_b, device);
	SPI_Write16Bit(AD9834_PHASE_REGISTER_ADDR ,0x00, device);
	switch(mode) {
		case SINE: SPI_Write16Bit(AD9834_EXIT_RESET ,OPBITEN, device); break;
		case TRIANGLE: SPI_Write16Bit(AD9834_EXIT_RESET, MODE, device); break;
		case SQUARE: SPI_Write16Bit(AD9834_EXIT_RESET, OPBITEN | DIV2, device); break;
		default: SPI_Write16Bit(AD9834_EXIT_RESET, SLEEP1, device); break;
	}
	SPI_InitAll();
	FG_SelectOutputType(device, mode);	
}

int FG_SetAmplitude(uint16_t valueIn, enum Device device) {
	uint16_t value = 4095 - valueIn;
	uint8_t dac_msb = ((value >> 8) & 0x0F);
	uint8_t dac_lsb = value & 0xFF;
	if (device == FG0)		SPI_Write16Bit(AMPLITUDE_A_ADDR | dac_msb, dac_lsb, DACA);
	else if (device == FG1) SPI_Write16Bit(AMPLITUDE_B_ADDR | dac_msb, dac_lsb, DACB);
	else return -1; 
	return 0;
}

bool Main_PollSwitch() {
	if (!(PB_PIN & S_INT)) {
		while(!(PB_PIN & S_INT));
		Buzzer_Beep();
		return true; 
	}
	else return false; 	
}

EncoderState Main_PollEncoder() {
	EncoderState state = NONE;
	volatile bool currentB = PIN_ENCODER & ENCODER_B;
	volatile bool currentA = PIN_ENCODER & ENCODER_A;
	/* State change */
	if ((Encoder.previousA && Encoder.previousB) && (!currentA && currentB)) Encoder.encoderSeqCntCW++;
	else if ((!Encoder.previousA && Encoder.previousB) && (!currentA && !currentB)) Encoder.encoderSeqCntCW++;
	else if ((!Encoder.previousA && !Encoder.previousB) && (currentA && !currentB)) Encoder.encoderSeqCntCW++;
	else if ((Encoder.previousA && !Encoder.previousB) && (currentA && currentB))Encoder.encoderSeqCntCW++;
	
	else if ((Encoder.previousA && Encoder.previousB) && (currentA && !currentB)) Encoder.encoderSeqCntCCW++;
	else if ((Encoder.previousA && !Encoder.previousB) && (!currentA && !currentB)) Encoder.encoderSeqCntCCW++;
	else if ((!Encoder.previousA && !Encoder.previousB) && (!currentA && currentB)) Encoder.encoderSeqCntCCW++;
	else if ((!Encoder.previousA && Encoder.previousB) && (currentA && currentB)) Encoder.encoderSeqCntCCW++;
	_delay_ms(1);
	
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
	
	return state;
}

void LCD_BrightnessAnimation() {
	for (uint8_t ix = 0; ix < 101; ix = ix + 1) {
		LCD_SetBrightness(ix);
		_delay_ms(10);
	}
}

void LCD_SetContrast(uint8_t value) {
	uint8_t final_value = (value * MAX_8BIT) / 100;
	SPI_Write16Bit(CONTRAST_ADDR,255 - final_value,LCD_POT);
}

int LCD_SetBrightness(uint8_t value) {
	if (value > 100) return -1;
	value = 75 + ((value * 25) / 100);
	uint16_t transformed_val = (value * DAC_DC_RATIO);
	uint8_t byte_a = (transformed_val >> 8) & 0x0F;
	uint8_t byte_b = transformed_val & 0xFF; 
	SPI_Write16Bit(VOLUME_ADDR | byte_a ,byte_b ,DACB);
	return 0;
}

int FG_SetBiasDC(enum Device device, uint16_t value, bool sign) {
	uint8_t reg_msb = (value >> 8) & 0x0F;
	uint8_t reg_lsb = value & 0xFF;
		switch(device) {
			case FG0:
				if (sign) SPI_Write16Bit(BIAS_A_NEG_ADDR | reg_msb, reg_lsb, DACA_BIAS);
				else	  SPI_Write16Bit(BIAS_A_POS_ADDR | reg_msb, reg_lsb, DACA_BIAS);
				break;
			case FG1:
				if (sign) SPI_Write16Bit(BIAS_B_NEG_ADDR | reg_msb, reg_lsb, DACB_BIAS);
				else	  SPI_Write16Bit(BIAS_B_POS_ADDR | reg_msb, reg_lsb, DACB_BIAS);
				break;
			default: return -1;	// Wrong device
				break;
		}
	return 0;
	}

void EEPROM_Erase_1K() {
	for (uint16_t ptr = 0; ptr < EEPROM_ADRESS_SPAN + 1; ptr++) eeprom_write_byte((uint8_t *)ptr, 0);
}

void Power_UpdateBatteryStatus() {
	ADMUX = 0;
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
	PowerStatus.battery_voltage = ADC * 0.547;
}

void Power_UpdateAcStatus() {
	ADMUX |= 1;
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
	uint16_t adc = ADC;
	if (adc > POWER_ADC_THRESHOLD) PowerStatus.ac_power_PowerStatus = true;
	else PowerStatus.ac_power_PowerStatus = false;
}

void Init_ClearWaveformValues(FGX FunctionGenerator) {
	FunctionGenerator.frequency_A = 0; 
	FunctionGenerator.frequency_B = 0;
	FunctionGenerator.amplitude_A = 0; 
	FunctionGenerator.amplitude_B = 0;
	FunctionGenerator.output_type_A = OFF; 
	FunctionGenerator.output_type_B = OFF;
	FunctionGenerator.bias_A = 0; 
	FunctionGenerator.bias_B = 0;
	FunctionGenerator.bias_A_sign = POSITIVE;
	FunctionGenerator.bias_B_sign = POSITIVE;
}

void Init_ClearUIValues() {
	memset(UI.frequency_A, '0', 7);
	memset(UI.frequency_B, '0', 7);
	memset(UI.amplitude_A, '0', 2);
	memset(UI.amplitude_B, '0', 2);
	
	memcpy(UI.type_A, "OFF", 3);
	memcpy(UI.type_B, "OFF", 3);
	
	memset(UI.bias_A, '0', 3);
	memset(UI.bias_B, '0', 3);
	
	UI.bias_A_sign = '+';
	UI.bias_B_sign = '+';
	
	memcpy(UI.lcd_contrast, "100", 3);
	memcpy(UI.lcd_brightness, "100", 3);
	memcpy(UI.batteryPowerStatus, "000", 3);
}

void Init_ClearLCDParameterValues() {
	LCD.brightness = 100;
	LCD.contrast = 100;
}

void Handle_LCD(MainScreen screen, DisplayPointer displayPointer, MainDeviceState mainDeviceState, FGX FunctionGenerator) {
	
	static bool staticLCDLoaded = false;
	
	if (screen != PARAMS_SCREEN) staticLCDLoaded = false;
	
	if (mainDeviceState == MENU_POINTER_ON) {
		switch(displayPointer) {
			case PTR_NULL:
			LCD_PrintChar(' ', LCD_LINE_1, 0);
			LCD_PrintChar(' ', LCD_LINE_2, 0);
			LCD_PrintChar(' ', LCD_LINE_3, 0);
			LCD_PrintChar(' ', LCD_LINE_4, 0);
			break;
			
			case PTR_TYPE_A: case PTR_TYPE_B: 
			LCD_PrintChar('>', LCD_LINE_1, 0); 
			LCD_PrintChar(' ', LCD_LINE_2, 0);
			LCD_PrintChar(' ', LCD_LINE_3, 0);
			LCD_PrintChar(' ', LCD_LINE_4, 0);
			break;
			
			case PTR_AMP_A: case PTR_AMP_B: case PTR_SAVE_PROF: case PTR_BRIGHT:
			LCD_PrintChar(' ', LCD_LINE_1, 0);
			LCD_PrintChar('>', LCD_LINE_2, 0);
			LCD_PrintChar(' ', LCD_LINE_3, 0);
			LCD_PrintChar(' ', LCD_LINE_4, 0);
			break;
			
			case PTR_FREQ_A: case PTR_FREQ_B: case PTR_SETT: case PTR_LOAD_PROF: case PTR_CONTR:
			LCD_PrintChar(' ', LCD_LINE_1, 0);
			LCD_PrintChar(' ', LCD_LINE_2, 0);
			LCD_PrintChar('>', LCD_LINE_3, 0);
			LCD_PrintChar(' ', LCD_LINE_4, 0);
			break;
			
			case PTR_BIAS_A: case PTR_BIAS_B: case PTR_SHUTDOWN: case PTR_BACK:
			LCD_PrintChar(' ', LCD_LINE_1, 0);
			LCD_PrintChar(' ', LCD_LINE_2, 0);
			LCD_PrintChar(' ', LCD_LINE_3, 0);
			LCD_PrintChar('>', LCD_LINE_4, 0);
			break;
		}
	}
	
	else if (mainDeviceState == PARAMETER_POINTER_ON) {
		switch(displayPointer) {
			case PTR_TYPE_A:
			switch(FunctionGenerator.output_type_A) {
				case SINE:
				UI.type_A[0] = 'S';
				UI.type_A[1] = 'I';
				UI.type_A[2] = 'N';
				break;
				
				case TRIANGLE:
				UI.type_A[0] = 'T';
				UI.type_A[1] = 'R';
				UI.type_A[2] = 'N';
				break;
				
				case SQUARE:
				UI.type_A[0] = 'S';
				UI.type_A[1] = 'Q';
				UI.type_A[2] = 'R';
				break;
				
				case DC:
				UI.type_A[0] = ' ';
				UI.type_A[1] = 'D';
				UI.type_A[2] = 'C';
				break;
				
				case OFF:
				UI.type_A[0] = 'O';
				UI.type_A[1] = 'F';
				UI.type_A[2] = 'F';
				break;
			}
			LCD_PrintChar(UI.type_A[0], LCD_LINE_1, 17);
			LCD_PrintChar(UI.type_A[1], LCD_LINE_1, 18);
			LCD_PrintChar(UI.type_A[2], LCD_LINE_1, 19);
			break;
		
			case PTR_TYPE_B:
			switch(FunctionGenerator.output_type_B) {
				case SINE:
				UI.type_B[0] = 'S';
				UI.type_B[1] = 'I';
				UI.type_B[2] = 'N';
				break;
				
				case TRIANGLE:
				UI.type_B[0] = 'T';
				UI.type_B[1] = 'R';
				UI.type_B[2] = 'N';
				break;
				
				case SQUARE:
				UI.type_B[0] = 'S';
				UI.type_B[1] = 'Q';
				UI.type_B[2] = 'R';
				break;
				
				case DC:
				UI.type_B[0] = ' ';
				UI.type_B[1] = 'D';
				UI.type_B[2] = 'C';
				break;
				
				case OFF:
				UI.type_B[0] = 'O';
				UI.type_B[1] = 'F';
				UI.type_B[2] = 'F';
				break;
			}
			
			LCD_PrintChar(UI.type_B[0], LCD_LINE_1, 17);
			LCD_PrintChar(UI.type_B[1], LCD_LINE_1, 18);
			LCD_PrintChar(UI.type_B[2], LCD_LINE_1, 19);
			break;
			
			case PTR_AMP_A:
			Main_UintToString(FunctionGenerator.amplitude_A, &UI.amplitude_A[0], 2);
			LCD_PrintChar(UI.amplitude_A[0], LCD_LINE_2, 14);
			LCD_PrintChar(UI.amplitude_A[1], LCD_LINE_2, 15);
			break;
				
			case PTR_AMP_B:
			Main_UintToString(FunctionGenerator.amplitude_B, &UI.amplitude_B[0], 2);
			LCD_PrintChar(UI.amplitude_B[0], LCD_LINE_2, 14);
			LCD_PrintChar(UI.amplitude_B[1], LCD_LINE_2, 15);
			break;
			
			case PTR_FREQ_A:
			Main_UintToString(FunctionGenerator.frequency_A, &UI.frequency_A[0], 7);
			LCD_PrintChar(UI.frequency_A[0], LCD_LINE_3, 7);
			LCD_PrintChar(UI.frequency_A[1], LCD_LINE_3, 9);
			LCD_PrintChar(UI.frequency_A[2], LCD_LINE_3, 10);
			LCD_PrintChar(UI.frequency_A[3], LCD_LINE_3, 11);
			LCD_PrintChar(UI.frequency_A[4], LCD_LINE_3, 13);
			LCD_PrintChar(UI.frequency_A[5], LCD_LINE_3, 14);
			LCD_PrintChar(UI.frequency_A[6], LCD_LINE_3, 15);
			break;
			
			case PTR_FREQ_B:
			Main_UintToString(FunctionGenerator.frequency_B, &UI.frequency_B[0], 7);
			LCD_PrintChar(UI.frequency_B[0], LCD_LINE_3, 7);
			LCD_PrintChar(UI.frequency_B[1], LCD_LINE_3, 9);
			LCD_PrintChar(UI.frequency_B[2], LCD_LINE_3, 10);
			LCD_PrintChar(UI.frequency_B[3], LCD_LINE_3, 11);
			LCD_PrintChar(UI.frequency_B[4], LCD_LINE_3, 13);
			LCD_PrintChar(UI.frequency_B[5], LCD_LINE_3, 14);
			LCD_PrintChar(UI.frequency_B[6], LCD_LINE_3, 15);
			break;
			
			case PTR_BIAS_A: 
			if (FunctionGenerator.bias_A_sign == POSITIVE) UI.bias_A_sign = '+';
			else UI.bias_A_sign = '-';
			Main_UintToString(abs(FunctionGenerator.bias_A), &UI.bias_A[0], 3);
			LCD_PrintChar(UI.bias_A_sign, LCD_LINE_4, 7);
			LCD_PrintChar(UI.bias_A[0], LCD_LINE_4, 8);
			LCD_PrintChar(UI.bias_A[1], LCD_LINE_4, 10);
			LCD_PrintChar(UI.bias_A[2], LCD_LINE_4, 11);
			break;
			
			case PTR_BIAS_B:
			if (FunctionGenerator.bias_B_sign == POSITIVE) UI.bias_B_sign = '+';
			else UI.bias_B_sign = '-';
			Main_UintToString(abs(FunctionGenerator.bias_B), &UI.bias_B[0], 3);
			LCD_PrintChar(UI.bias_B_sign, LCD_LINE_4, 7);
			LCD_PrintChar(UI.bias_B[0], LCD_LINE_4, 8);
			LCD_PrintChar(UI.bias_B[1], LCD_LINE_4, 10);
			LCD_PrintChar(UI.bias_B[2], LCD_LINE_4, 11);
			break;
			
			default: break;			
			
		}
	}
				
	else if (mainDeviceState == PARAMETER_LCD_POINTER_ON) {
		switch(displayPointer) {
			case PTR_BRIGHT:			
			Main_UintToString(LCD.brightness, &UI.lcd_brightness[0], 3);
			LCD_PrintChar(UI.lcd_brightness[0], LCD_LINE_2, 13);
			LCD_PrintChar(UI.lcd_brightness[1], LCD_LINE_2, 14);
			LCD_PrintChar(UI.lcd_brightness[2], LCD_LINE_2, 15);
			break;

			case PTR_CONTR:
			Main_UintToString(LCD.contrast, &UI.lcd_contrast[0], 3);
			LCD_PrintChar(UI.lcd_contrast[0], LCD_LINE_3, 13);
			LCD_PrintChar(UI.lcd_contrast[1], LCD_LINE_3, 14);
			LCD_PrintChar(UI.lcd_contrast[2], LCD_LINE_3, 15);
			break;
			
			default: break;
		}
	}
	
	else {
		switch(screen) {
			case MAIN_SCREEN_A:
			LCD_PrintLine(LCD_MAIN_STRINGA_1, LCD_LINE_1);
			LCD_PrintLine(LCD_MAIN_STRING_2, LCD_LINE_2);
			LCD_PrintLine(LCD_MAIN_STRING_3, LCD_LINE_3);
			LCD_PrintLine(LCD_MAIN_STRING_4, LCD_LINE_4);
			
			LCD_PrintChar(UI.type_A[0], LCD_LINE_1, 17);
			LCD_PrintChar(UI.type_A[1], LCD_LINE_1, 18);
			LCD_PrintChar(UI.type_A[2], LCD_LINE_1, 19);
			
			LCD_PrintChar(UI.amplitude_A[0], LCD_LINE_2, 14);
			LCD_PrintChar(UI.amplitude_A[1], LCD_LINE_2, 15);
			
			LCD_PrintChar(UI.frequency_A[0], LCD_LINE_3, 7);
			LCD_PrintChar(UI.frequency_A[1], LCD_LINE_3, 9);
			LCD_PrintChar(UI.frequency_A[2], LCD_LINE_3, 10);
			LCD_PrintChar(UI.frequency_A[3], LCD_LINE_3, 11);
			LCD_PrintChar(UI.frequency_A[4], LCD_LINE_3, 13);
			LCD_PrintChar(UI.frequency_A[5], LCD_LINE_3, 14);
			LCD_PrintChar(UI.frequency_A[6], LCD_LINE_3, 15);
			
			LCD_PrintChar(UI.bias_A_sign, LCD_LINE_4, 7);
			LCD_PrintChar(UI.bias_A[0], LCD_LINE_4, 8);
			LCD_PrintChar(UI.bias_A[1], LCD_LINE_4, 10);
			LCD_PrintChar(UI.bias_A[2], LCD_LINE_4, 11);
			break;
			
			case MAIN_SCREEN_B:
			LCD_PrintLine(LCD_MAIN_STRINGB_1, LCD_LINE_1);
			LCD_PrintLine(LCD_MAIN_STRING_2, LCD_LINE_2);
			LCD_PrintLine(LCD_MAIN_STRING_3, LCD_LINE_3);
			LCD_PrintLine(LCD_MAIN_STRING_4, LCD_LINE_4);
			
			LCD_PrintChar(UI.type_B[0], LCD_LINE_1, 17);
			LCD_PrintChar(UI.type_B[1], LCD_LINE_1, 18);
			LCD_PrintChar(UI.type_B[2], LCD_LINE_1, 19);
			
			LCD_PrintChar(UI.amplitude_B[0], LCD_LINE_2, 14);
			LCD_PrintChar(UI.amplitude_B[1], LCD_LINE_2, 15);
			
			LCD_PrintChar(UI.frequency_B[0], LCD_LINE_3, 7);
			LCD_PrintChar(UI.frequency_B[1], LCD_LINE_3, 9);
			LCD_PrintChar(UI.frequency_B[2], LCD_LINE_3, 10);
			LCD_PrintChar(UI.frequency_B[3], LCD_LINE_3, 11);
			LCD_PrintChar(UI.frequency_B[4], LCD_LINE_3, 13);
			LCD_PrintChar(UI.frequency_B[5], LCD_LINE_3, 14);
			LCD_PrintChar(UI.frequency_B[6], LCD_LINE_3, 15);
			
			LCD_PrintChar(UI.bias_B_sign, LCD_LINE_4, 7);
			LCD_PrintChar(UI.bias_B[0], LCD_LINE_4, 8);
			LCD_PrintChar(UI.bias_B[1], LCD_LINE_4, 10);
			LCD_PrintChar(UI.bias_B[2], LCD_LINE_4, 11);
			break;
			
			case PARAMS_SCREEN:
			if (!staticLCDLoaded) {
				staticLCDLoaded = true;
				LCD_PrintLine(LCD_MAIN_SETTINGS_STRING_1, LCD_LINE_1);
				LCD_PrintLine(LCD_MAIN_SETTINGS_STRING_2, LCD_LINE_2);
				LCD_PrintLine(LCD_MAIN_SETTINGS_STRING_3, LCD_LINE_3);
				LCD_PrintLine(LCD_MAIN_SETTINGS_STRING_4, LCD_LINE_4);
			}
			
			Main_UintToString(PowerStatus.battery_voltage, &UI.batteryPowerStatus[0], 3);
			LCD_PrintChar(UI.batteryPowerStatus[0], LCD_LINE_1, 13);
			LCD_PrintChar(UI.batteryPowerStatus[1], LCD_LINE_1, 15);
			LCD_PrintChar(UI.batteryPowerStatus[2], LCD_LINE_1, 16);
			
			if (PowerStatus.ac_power_PowerStatus) {
				LCD_PrintChar('O', LCD_LINE_2, 16);
				LCD_PrintChar('N', LCD_LINE_2, 17);
				LCD_PrintChar(' ', LCD_LINE_2, 18);
			}
			else if (!PowerStatus.ac_power_PowerStatus) {
				LCD_PrintChar('O', LCD_LINE_2, 16);
				LCD_PrintChar('F', LCD_LINE_2, 17);
				LCD_PrintChar('F', LCD_LINE_2, 18);
			}
			break;
			
			case PROFILE_SCREEN:
			LCD_PrintLine(LCD_PROFILE_SETTINGS_STRING_1, LCD_LINE_1);
			LCD_PrintLine(LCD_PROFILE_SETTINGS_STRING_2, LCD_LINE_2);
			LCD_PrintLine(LCD_PROFILE_SETTINGS_STRING_3, LCD_LINE_3);
			LCD_PrintLine(LCD_PROFILE_SETTINGS_STRING_4, LCD_LINE_4);
			break;

			case LCD_SCREEN:
			LCD_PrintLine(LCD_SCREEN_SETTINGS_STRING_1, LCD_LINE_1);
			LCD_PrintLine(LCD_SCREEN_SETTINGS_STRING_2, LCD_LINE_2);
			LCD_PrintLine(LCD_SCREEN_SETTINGS_STRING_3, LCD_LINE_3);
			LCD_PrintLine(LCD_SCREEN_SETTINGS_STRING_4, LCD_LINE_4);
			
			Main_UintToString(LCD.brightness, &UI.lcd_brightness[0], 3);
			Main_UintToString(LCD.contrast, &UI.lcd_contrast[0], 3);
			
			LCD_PrintChar(UI.lcd_brightness[0], LCD_LINE_2, 13);
			LCD_PrintChar(UI.lcd_brightness[1], LCD_LINE_2, 14);
			LCD_PrintChar(UI.lcd_brightness[2], LCD_LINE_2, 15);
			
			LCD_PrintChar(UI.lcd_contrast[0], LCD_LINE_3, 13);
			LCD_PrintChar(UI.lcd_contrast[1], LCD_LINE_3, 14);
			LCD_PrintChar(UI.lcd_contrast[2], LCD_LINE_3, 15);
			break;
			
			default: break;
		}					
	}
}

void Handle_LCDParameter(DisplayPointer displayPointer) {
	switch(displayPointer) {
		case PTR_BRIGHT: LCD_SetBrightness(LCD.brightness); break;
		case PTR_CONTR: LCD_SetContrast(LCD.contrast); break;
		default: break;
	}
}

void Main_UintToString(uint32_t number, char *string, uint8_t length) {
	uint8_t iPtr = 0;
	while(iPtr < length) {
		string[length - iPtr - 1] = (number % 10) + '0';
		number /= 10;
		iPtr++;
	}
}

void LCD_LogoDisplay() {
	char lbuff[20];
	LCD_Clear();
	LCD_PrintLine("<<Portable FuncGen>>", LCD_LINE_1);
	snprintf(lbuff, 20, " Firmware V%c.%c.%c ", FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR, FIRMWARE_VERSION_BUILD);
	LCD_PrintLine(lbuff, LCD_LINE_2);
	LCD_PrintLine("  <Initializing...> ", LCD_LINE_4);
	_delay_ms(500);
}

void EEPROM_SaveProfile(FGX FunctionGenerator) {
	eeprom_write_dword((uint32_t *)0x00, FunctionGenerator.frequency_A); eeprom_busy_wait();
	eeprom_write_dword((uint32_t *)0x04, FunctionGenerator.frequency_B); eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)0x08, FunctionGenerator.amplitude_A); eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)0x09, FunctionGenerator.amplitude_B); eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)0x0A, FunctionGenerator.bias_A_sign); eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)0x0B, FunctionGenerator.bias_B_sign); eeprom_busy_wait();
	eeprom_write_word((uint16_t *)0x0C, abs(FunctionGenerator.bias_A)); eeprom_busy_wait();
	eeprom_write_word((uint16_t *)0x0E, abs(FunctionGenerator.bias_B)); eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)0x10, FunctionGenerator.output_type_A); eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)0x11, FunctionGenerator.output_type_B); eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)0x12, LCD.brightness); eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)0x13, LCD.contrast); eeprom_busy_wait();
}

void EEPROM_LoadProfile(FGX FunctionGenerator) {
	FunctionGenerator.frequency_A = eeprom_read_dword((uint32_t *)0x00); eeprom_busy_wait();
	FunctionGenerator.frequency_B = eeprom_read_dword((uint32_t *)0x04); eeprom_busy_wait();
	FunctionGenerator.amplitude_A = eeprom_read_byte((uint8_t *)0x08); eeprom_busy_wait();
	FunctionGenerator.amplitude_B = eeprom_read_byte((uint8_t *)0x09); eeprom_busy_wait();
	FunctionGenerator.bias_A_sign = eeprom_read_byte((uint8_t *)0x0A); eeprom_busy_wait();
	FunctionGenerator.bias_B_sign = eeprom_read_byte((uint8_t *)0x0B); eeprom_busy_wait();
	
	if (FunctionGenerator.bias_A_sign == POSITIVE) { FunctionGenerator.bias_A = eeprom_read_word((uint16_t *)0x0C); eeprom_busy_wait(); }
	else FunctionGenerator.bias_A = -abs(eeprom_read_word((uint16_t *)0x0C)); eeprom_busy_wait();
	
	if (FunctionGenerator.bias_B_sign == POSITIVE) { FunctionGenerator.bias_B = eeprom_read_word((uint16_t *)0x0E); eeprom_busy_wait(); }
	else FunctionGenerator.bias_A = -abs(eeprom_read_word((uint16_t *)0x0E)); eeprom_busy_wait();
	
	FunctionGenerator.output_type_A = eeprom_read_byte((uint8_t *)0x10); eeprom_busy_wait();
	FunctionGenerator.output_type_B = eeprom_read_byte((uint8_t *)0x11); eeprom_busy_wait();
	LCD.brightness = eeprom_read_byte((uint8_t *)0x12); eeprom_busy_wait();
	LCD.contrast = eeprom_read_byte((uint8_t *)0x13); eeprom_busy_wait();
	
	Main_UintToString(LCD.brightness, &UI.lcd_brightness[0], 3);
	Main_UintToString(LCD.contrast, &UI.lcd_contrast[0], 3);
	Main_UintToString(abs(FunctionGenerator.bias_A), &UI.bias_A[0], 3);
	Main_UintToString(abs(FunctionGenerator.bias_B), &UI.bias_B[0], 3);
	Main_UintToString(FunctionGenerator.frequency_B, &UI.frequency_B[0], 7);
	Main_UintToString(FunctionGenerator.frequency_A, &UI.frequency_A[0], 7);
	Main_UintToString(FunctionGenerator.amplitude_B, &UI.amplitude_B[0], 2);
	Main_UintToString(FunctionGenerator.amplitude_A, &UI.amplitude_A[0], 2);
	if (FunctionGenerator.bias_B_sign == POSITIVE) UI.bias_B_sign = '+';
	else UI.bias_B_sign = '-';
	if (FunctionGenerator.bias_A_sign == POSITIVE) UI.bias_A_sign = '+';
	else UI.bias_A_sign = '-';
	switch(FunctionGenerator.output_type_B) {
		case SINE:UI.type_B[0] = 'S'; UI.type_B[1] = 'I'; UI.type_B[2] = 'N'; break;
		case TRIANGLE:UI.type_B[0] = 'T'; UI.type_B[1] = 'R'; UI.type_B[2] = 'N'; break;
		case SQUARE: UI.type_B[0] = 'S'; UI.type_B[1] = 'Q'; UI.type_B[2] = 'R'; break;
		case DC: UI.type_B[0] = ' '; UI.type_B[1] = 'D'; UI.type_B[2] = 'C'; break;
		case OFF: UI.type_B[0] = 'O'; UI.type_B[1] = 'F'; UI.type_B[2] = 'F'; break;
	}
	switch(FunctionGenerator.output_type_A) {
		case SINE:UI.type_A[0] = 'S'; UI.type_A[1] = 'I'; UI.type_A[2] = 'N'; break;
		case TRIANGLE: UI.type_A[0] = 'T'; UI.type_A[1] = 'R'; UI.type_A[2] = 'N'; break;
		case SQUARE: UI.type_A[0] = 'S'; UI.type_A[1] = 'Q'; UI.type_A[2] = 'R'; break;
		case DC: UI.type_A[0] = ' '; UI.type_A[1] = 'D'; UI.type_A[2] = 'C'; break;
		case OFF: UI.type_A[0] = 'O'; UI.type_A[1] = 'F'; UI.type_A[2] = 'F'; break;
	}
	
	FG_SetFunction(FG0, FunctionGenerator.frequency_A, FunctionGenerator.output_type_A);
	FG_SetFunction(FG1, FunctionGenerator.frequency_B, FunctionGenerator.output_type_B);
	FG_SetAmplitude(FunctionGenerator.amplitude_A * 58.5, FG0);
	FG_SetAmplitude(FunctionGenerator.amplitude_B * 58.5, FG1);
	
	if (FunctionGenerator.bias_A_sign == POSITIVE) FG_SetBiasDC(FG0, 0, NEGATIVE);
	else FG_SetBiasDC(FG0, 0, POSITIVE);
	FG_SetBiasDC(FG0, (uint16_t)(abs(FunctionGenerator.bias_A) * MAX_12BIT / 330), FunctionGenerator.bias_A_sign);
	
	if (FunctionGenerator.bias_A_sign == POSITIVE) FG_SetBiasDC(FG0, 0, NEGATIVE);
	else FG_SetBiasDC(FG0, 0, POSITIVE);
	FG_SetBiasDC(FG0, (uint16_t)(abs(FunctionGenerator.bias_A) * MAX_12BIT / 330), FunctionGenerator.bias_A_sign);
	
	LCD_SetContrast(LCD.contrast);
	LCD_SetBrightness(LCD.brightness);
}

void Init_FG() {
	FG_SetFunction(FG0, 0, OFF);
	FG_SetFunction(FG1, 0, OFF);
	FG_SetAmplitude(0, FG0);
	FG_SetAmplitude(0, FG1);
	FG_SetBiasDC(FG0, 0, NEGATIVE);
	FG_SetBiasDC(FG0, 0, POSITIVE);
	FG_SetBiasDC(FG1, 0, NEGATIVE);
	FG_SetBiasDC(FG1, 0, POSITIVE);
}


int main() {
	/* Testing definitions */
	#ifdef PRE_PROG
		ENABLE_DEVICE();
		while(1);
	#endif
	
	Init_Device();
	Init_UI();
	Init_FG();

	EncoderState encoderState = NONE;
	bool switchState = false;
	
	static MainDeviceState mainDeviceState = PRIMARY_SCREENS;
	static FGX FunctionGenerator;
	
	static Screen display;
	static DisplayPointer displayPointer;
	
	static uint8_t buttonPressCounter = 0;
	
	static uint16_t prevBatVoltage = 0;
	static bool prevAcPowerStatus = false;
	
	static bool lcdFunctionChanged = false;

	/* Initialization sequence */
	display.mainScreen = MAIN_SCREEN_A;
	display.stateChanged = false;
	buttonPressCounter = 0;
	
	Init_ClearWaveformValues(FunctionGenerator);
	Init_ClearUIValues();
	Init_ClearLCDParameterValues();
	Handle_LCD(display.mainScreen, PTR_NULL, mainDeviceState, FunctionGenerator);
	
	while(1) {
		encoderState = Main_PollEncoder();
		switchState = Main_PollSwitch();
		
		if (display.stateChanged) {
			display.stateChanged = false;
			Handle_LCD(display.mainScreen, displayPointer, mainDeviceState, FunctionGenerator);
		}
		
		if (lcdFunctionChanged) {
			lcdFunctionChanged = false;
			Handle_LCDParameter(displayPointer);
		}
		
		if (mainDeviceState == PRIMARY_SCREENS) {
			switch(display.mainScreen) {
				case MAIN_SCREEN_A:
					if (encoderState == CW) {
						display.mainScreen = MAIN_SCREEN_B;
						display.stateChanged = true;
					}
					
					else if (encoderState == CCW) {
						display.mainScreen = PARAMS_SCREEN;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = MENU_POINTER_ON;
						displayPointer = PTR_TYPE_A;
						display.stateChanged = true;
					}
					break;
					
				case MAIN_SCREEN_B:
					if (encoderState == CW) {
						display.mainScreen = PARAMS_SCREEN;
						display.stateChanged = true;
					}
					
					else if (encoderState == CCW) {
						display.mainScreen = MAIN_SCREEN_A;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = MENU_POINTER_ON;
						displayPointer = PTR_TYPE_B;
						display.stateChanged = true;
					}
					break;
					
				case PARAMS_SCREEN:
					if (encoderState == CW) {
						display.mainScreen = MAIN_SCREEN_A;
						display.stateChanged = true;
					}
					
					else if (encoderState == CCW) {
						display.mainScreen = MAIN_SCREEN_B;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = MENU_POINTER_ON;
						displayPointer = PTR_SETT;
						display.stateChanged = true;
					}
					Power_UpdateAcStatus();
					Power_UpdateBatteryStatus();
					if ((PowerStatus.battery_voltage <  prevBatVoltage + 1) \
					|| (PowerStatus.battery_voltage >  prevBatVoltage + 1)) {
						display.stateChanged = true;
						prevBatVoltage = PowerStatus.battery_voltage;
					}
					
					if (PowerStatus.ac_power_PowerStatus != prevAcPowerStatus) {
						display.stateChanged = true;
						prevAcPowerStatus = PowerStatus.ac_power_PowerStatus;
					}
					break;	
					
				case PROFILE_SCREEN:
					if (encoderState == CW) {
						display.mainScreen = LCD_SCREEN;
						display.stateChanged = true;
					}
					
					else if (encoderState == CCW) {
						display.mainScreen = LCD_SCREEN;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = MENU_POINTER_ON;
						displayPointer = PTR_SAVE_PROF;
						display.stateChanged = true;
					}
					
					break;		
							
				case LCD_SCREEN:
					if (encoderState == CW) {
						display.mainScreen = PROFILE_SCREEN;
						display.stateChanged = true;
					}
					
					else if (encoderState == CCW) {
						display.mainScreen = PROFILE_SCREEN;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = MENU_POINTER_ON;
						displayPointer = PTR_BRIGHT;
						display.stateChanged = true;
					}
					
					break;				
						
				default: break;
				
			}
		}
		else if (mainDeviceState == MENU_POINTER_ON) { 
			switch(displayPointer) {
				case PTR_NULL: break;
				/* Channel A Block */
				case PTR_TYPE_A: 
					if (encoderState == CW) {
						displayPointer = PTR_AMP_A;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_BIAS_A;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PARAMETER_POINTER_ON;
						display.stateChanged = true;
					}
					break;
				case PTR_AMP_A:
					if (encoderState == CW) {
						displayPointer = PTR_FREQ_A;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_TYPE_A;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PARAMETER_POINTER_ON;
						display.stateChanged = true;
					}
					break;
				case PTR_FREQ_A:
					if (encoderState == CW) {
						displayPointer = PTR_BIAS_A;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_AMP_A;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PARAMETER_POINTER_ON;
						display.stateChanged = true;
					}
					break;
				case PTR_BIAS_A:
					if (encoderState == CW) {
						displayPointer = PTR_TYPE_A;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_FREQ_A;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PARAMETER_POINTER_ON;
						display.stateChanged = true;
					}
					break;
				/* Channel B Block */
				case PTR_TYPE_B:
					if (encoderState == CW) {
						displayPointer = PTR_AMP_B;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_BIAS_B;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PARAMETER_POINTER_ON;
						display.stateChanged = true;
					}
					break;
				case PTR_AMP_B:
					if (encoderState == CW) {
						displayPointer = PTR_FREQ_B;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_TYPE_B;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PARAMETER_POINTER_ON;
						display.stateChanged = true;
					}
					break;
				case PTR_FREQ_B:
					if (encoderState == CW) {
						displayPointer = PTR_BIAS_B;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_AMP_B;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PARAMETER_POINTER_ON;
						display.stateChanged = true;
					}
					break;
				case PTR_BIAS_B:
					if (encoderState == CW) {
						displayPointer = PTR_TYPE_B;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_FREQ_B;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PARAMETER_POINTER_ON;
						display.stateChanged = true;
					}
					break;
				case PTR_SETT:
					if (encoderState == CW) {
						displayPointer = PTR_SHUTDOWN;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_SHUTDOWN;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PRIMARY_SCREENS;
						display.mainScreen = PROFILE_SCREEN;
						display.stateChanged = true;
					}
					break;
					
				case PTR_SAVE_PROF:
					if (encoderState == CW) {
						displayPointer = PTR_LOAD_PROF;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_BACK;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PRIMARY_SCREENS;
						displayPointer = PTR_NULL;
						display.stateChanged = true;
						display.mainScreen = PARAMS_SCREEN;
						EEPROM_SaveProfile(FunctionGenerator);
						Buzzer_Beep();
					}
					break;
					
				case PTR_LOAD_PROF:
					if (encoderState == CW) {
						displayPointer = PTR_BACK;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_SAVE_PROF;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PRIMARY_SCREENS;
						displayPointer = PTR_NULL;
						display.stateChanged = true;
						display.mainScreen = PROFILE_SCREEN;
						EEPROM_LoadProfile(FunctionGenerator);
						Buzzer_Beep();
					}
					break;	
					
				case PTR_BACK:
					if (encoderState == CW) {
						displayPointer = PTR_SAVE_PROF;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_LOAD_PROF;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PRIMARY_SCREENS;
						displayPointer = PTR_NULL;
						display.stateChanged = true;
						display.mainScreen = PARAMS_SCREEN;
						
					}
					break;
				
				case PTR_BRIGHT:
					if (encoderState == CW) {
						displayPointer = PTR_CONTR;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_CONTR;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PARAMETER_LCD_POINTER_ON;
						display.stateChanged = true;
					}
					break;
				
				case PTR_CONTR:
					if (encoderState == CW) {
						displayPointer = PTR_BRIGHT;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_BRIGHT;
						display.stateChanged = true;
					}
					else if (switchState) {
						mainDeviceState = PARAMETER_LCD_POINTER_ON;
						display.stateChanged = true;
					}
					break;
												
				case PTR_SHUTDOWN:
					if (encoderState == CW) {
						displayPointer = PTR_SETT;
						display.stateChanged = true;
					}
					else if (encoderState == CCW) {
						displayPointer = PTR_SETT;
						display.stateChanged = true;
					}
					else if (switchState) Main_ShutdownDevice(false);
					break;
					
				default: break;
			}
		}
		else if (mainDeviceState == PARAMETER_POINTER_ON) {
			switch(displayPointer) {
				
				case PTR_NULL: break;
				
				case PTR_TYPE_A:
				if (encoderState == CW) {
					display.stateChanged = true;
					switch(FunctionGenerator.output_type_A) {
						case OFF: FunctionGenerator.output_type_A = SINE; break;
						case SINE: FunctionGenerator.output_type_A = TRIANGLE; break;
						case TRIANGLE: FunctionGenerator.output_type_A = SQUARE; break;
						case SQUARE: FunctionGenerator.output_type_A = DC; break;
						case DC: default: FunctionGenerator.output_type_A = OFF; break;
					}
					FG_SetFunction(FG0, FunctionGenerator.frequency_A, FunctionGenerator.output_type_A);
				}
				else if (encoderState == CCW) {
					display.stateChanged = true;
					switch(FunctionGenerator.output_type_A) {
						case OFF: FunctionGenerator.output_type_A = DC; break;
						case SINE: FunctionGenerator.output_type_A = OFF; break;
						case TRIANGLE: FunctionGenerator.output_type_A = SINE; break;
						case SQUARE: FunctionGenerator.output_type_A = TRIANGLE; break;
						case DC: default: FunctionGenerator.output_type_A = SQUARE; break;
					}
					FG_SetFunction(FG0, FunctionGenerator.frequency_A, FunctionGenerator.output_type_A);
				}
				
				else if (switchState) {
					mainDeviceState = PRIMARY_SCREENS;
					display.stateChanged = true;
					displayPointer = PTR_NULL;
				}
				
				break;
				
				case PTR_AMP_A:
				if (encoderState == CW) {
					display.stateChanged = true;
					if (FunctionGenerator.amplitude_A >= 70) FunctionGenerator.amplitude_A = 70;
					else {
						switch(buttonPressCounter) {
							case 0: FunctionGenerator.amplitude_A++; break;
							case 1: FunctionGenerator.amplitude_A += 10; break;
						}
					}
					FG_SetAmplitude(FunctionGenerator.amplitude_A * 58.5, FG0);
				}
				else if (encoderState == CCW) {
					display.stateChanged = true;
					if (FunctionGenerator.amplitude_A <= 0) FunctionGenerator.amplitude_A = 0;
					else {
						switch(buttonPressCounter) {
							case 0: FunctionGenerator.amplitude_A--; break;
							case 1: FunctionGenerator.amplitude_A -= 10; break;
						}
					}
					FG_SetAmplitude(FunctionGenerator.amplitude_A * 58.5, FG0);
				}
				
				else if (switchState) {
					if (buttonPressCounter < 1) buttonPressCounter++;
					else {
						buttonPressCounter = 0;
						mainDeviceState = PRIMARY_SCREENS;
						displayPointer = PTR_NULL;
					}
					display.stateChanged = true;
				}
				break;

				case PTR_FREQ_A:
				if (encoderState == CW) {
					display.stateChanged = true;
					if (FunctionGenerator.frequency_A >= 1000000) FunctionGenerator.frequency_A = 1000000;
					else {
						switch(buttonPressCounter) {
							case 0: FunctionGenerator.frequency_A++; break;
							case 1: FunctionGenerator.frequency_A += 10; break;
							case 2: FunctionGenerator.frequency_A += 100; break;
							case 3: FunctionGenerator.frequency_A += 1000; break;
							case 4: FunctionGenerator.frequency_A += 10000; break;
							case 5: FunctionGenerator.frequency_A += 100000; break;
						}
					}
					FG_SetFunction(FG0, FunctionGenerator.frequency_A, FunctionGenerator.output_type_A);
				}
				
				else if (encoderState == CCW) {
					display.stateChanged = true;
					if (FunctionGenerator.frequency_A <= 0) FunctionGenerator.frequency_A = 0;
					else {
						switch(buttonPressCounter) {
							case 0: FunctionGenerator.frequency_A--; break;
							case 1: FunctionGenerator.frequency_A -= 10; break;
							case 2: FunctionGenerator.frequency_A -= 100; break;
							case 3: FunctionGenerator.frequency_A -= 1000; break;
							case 4: FunctionGenerator.frequency_A -= 10000; break;
							case 5: FunctionGenerator.frequency_A -= 100000; break;
						}
					}
					FG_SetFunction(FG0, FunctionGenerator.frequency_A, FunctionGenerator.output_type_A);
				}
				
				else if (switchState) {
					if (buttonPressCounter < 5) buttonPressCounter++;
					else {
						buttonPressCounter = 0;
						mainDeviceState = PRIMARY_SCREENS;
						displayPointer = PTR_NULL;
				}
					display.stateChanged = true;
				}
				break;
				
				case PTR_BIAS_A:
				if (encoderState == CW) {
					display.stateChanged = true;
					if (FunctionGenerator.bias_A < 0) FunctionGenerator.bias_A_sign = NEGATIVE;
					else FunctionGenerator.bias_A_sign = POSITIVE;
					
					if (FunctionGenerator.bias_A >= 330) FunctionGenerator.bias_A = 330;
					else {
						switch(buttonPressCounter) {
							case 0: FunctionGenerator.bias_A++; break;
							case 1: FunctionGenerator.bias_A += 10; break;
							case 2: FunctionGenerator.bias_A += 100; break;
						}
					}
					
					if (FunctionGenerator.bias_A_sign == POSITIVE) FG_SetBiasDC(FG0, 0, NEGATIVE);
					else FG_SetBiasDC(FG0, 0, POSITIVE);
					FG_SetBiasDC(FG0, (uint16_t)(abs(FunctionGenerator.bias_A) * 12.4), FunctionGenerator.bias_A_sign);
				}
				
				else if (encoderState == CCW) {
					display.stateChanged = true;
					if (FunctionGenerator.bias_A < 0) FunctionGenerator.bias_A_sign = NEGATIVE;
					else FunctionGenerator.bias_A_sign = POSITIVE;
					
					if (FunctionGenerator.bias_A <= -330) FunctionGenerator.bias_A = -330;
					else {
						switch(buttonPressCounter) {
							case 0: FunctionGenerator.bias_A--; break;
							case 1: FunctionGenerator.bias_A -= 10; break;
							case 2: FunctionGenerator.bias_A -= 100; break;
						}
					}
					
					if (FunctionGenerator.bias_A_sign == POSITIVE) FG_SetBiasDC(FG0, 0, NEGATIVE);
					else FG_SetBiasDC(FG0, 0, POSITIVE);
					FG_SetBiasDC(FG0, (uint16_t)(abs(FunctionGenerator.bias_A) * 12.4), FunctionGenerator.bias_A_sign);
				}
				
				else if (switchState) {
					if (buttonPressCounter < 2) buttonPressCounter++;
					else {
						buttonPressCounter = 0;
						mainDeviceState = PRIMARY_SCREENS;
						displayPointer = PTR_NULL;
					}
					display.stateChanged = true;
				}
				break;
				
				case PTR_TYPE_B:
				if (encoderState == CW) {
					display.stateChanged = true;
					switch(FunctionGenerator.output_type_B) {
						case OFF: FunctionGenerator.output_type_B = SINE; break;
						case SINE: FunctionGenerator.output_type_B = TRIANGLE; break;
						case TRIANGLE: FunctionGenerator.output_type_B = SQUARE; break;
						case SQUARE: FunctionGenerator.output_type_B = DC; break;
						case DC: default: FunctionGenerator.output_type_B = OFF; break;
					}
					FG_SetFunction(FG1, FunctionGenerator.frequency_B, FunctionGenerator.output_type_B);
				}
				else if (encoderState == CCW) {
					display.stateChanged = true;
					switch(FunctionGenerator.output_type_B) {
						case OFF: FunctionGenerator.output_type_B = DC; break;
						case SINE: FunctionGenerator.output_type_B = OFF; break;
						case TRIANGLE: FunctionGenerator.output_type_B = SINE; break;
						case SQUARE: FunctionGenerator.output_type_B = TRIANGLE; break;
						case DC: default: FunctionGenerator.output_type_B = SQUARE; break;
					}
					FG_SetFunction(FG1, FunctionGenerator.frequency_B, FunctionGenerator.output_type_B);
				}
				
				else if (switchState) {
					mainDeviceState = PRIMARY_SCREENS;
					display.stateChanged = true;
					displayPointer = PTR_NULL;
				}
				
				break;
				
				case PTR_AMP_B:
				if (encoderState == CW) {
					display.stateChanged = true;
					if (FunctionGenerator.amplitude_B >= 70) FunctionGenerator.amplitude_B = 70;
					else {
						switch(buttonPressCounter) {
							case 0: FunctionGenerator.amplitude_B++; break;
							case 1: FunctionGenerator.amplitude_B += 10; break;
						}
					}
					FG_SetAmplitude(FunctionGenerator.amplitude_B * 58.5, FG1);
				}
				else if (encoderState == CCW) {
					display.stateChanged = true;
					if (FunctionGenerator.amplitude_B <= 0) FunctionGenerator.amplitude_B = 0;
					else {
						switch(buttonPressCounter) {
							case 0: FunctionGenerator.amplitude_B--; break;
							case 1: FunctionGenerator.amplitude_B -= 10; break;
						}
					}
					FG_SetAmplitude(FunctionGenerator.amplitude_B * 58.5, FG1);
				}
				
				else if (switchState) {
					if (buttonPressCounter < 1) buttonPressCounter++;
					else {
						buttonPressCounter = 0;
						mainDeviceState = PRIMARY_SCREENS;
						displayPointer = PTR_NULL;
					}
					display.stateChanged = true;
				}
				
				break;

				case PTR_FREQ_B:
				if (encoderState == CW) {
					display.stateChanged = true;
					if (FunctionGenerator.frequency_B >= 1000000) FunctionGenerator.frequency_B = 1000000;
					else {
						switch(buttonPressCounter) {
							case 0: FunctionGenerator.frequency_B++; break;
							case 1: FunctionGenerator.frequency_B += 10; break;
							case 2: FunctionGenerator.frequency_B += 100; break;
							case 3: FunctionGenerator.frequency_B += 1000; break;
							case 4: FunctionGenerator.frequency_B += 10000; break;
							case 5: FunctionGenerator.frequency_B += 100000; break;
							default: break;
						}
					}
					FG_SetFunction(FG1, FunctionGenerator.frequency_B, FunctionGenerator.output_type_B);									
				}
				else if (encoderState == CCW) {
					display.stateChanged = true;
					if (FunctionGenerator.frequency_B <= 0) FunctionGenerator.frequency_B = 0;
					else {
						switch(buttonPressCounter) {
							case 0: FunctionGenerator.frequency_B--; break;
							case 1: FunctionGenerator.frequency_B -= 10; break;
							case 2: FunctionGenerator.frequency_B -= 100; break;
							case 3: FunctionGenerator.frequency_B -= 1000; break;
							case 4: FunctionGenerator.frequency_B -= 10000; break;
							case 5: FunctionGenerator.frequency_B -= 100000; break;
							default: break;
						}
					}
					FG_SetFunction(FG1, FunctionGenerator.frequency_B, FunctionGenerator.output_type_B);
				}
					
				else if (switchState) {
					if (buttonPressCounter < 5) buttonPressCounter++;
					else {
						buttonPressCounter = 0;
						mainDeviceState = PRIMARY_SCREENS;
						displayPointer = PTR_NULL;
					}
					display.stateChanged = true;
				}
				break;
					
				case PTR_BIAS_B:
				if (encoderState == CW) {
					display.stateChanged = true;
					if (FunctionGenerator.bias_B < 0) FunctionGenerator.bias_B_sign = NEGATIVE;
					else FunctionGenerator.bias_B_sign = POSITIVE;
					
					if (FunctionGenerator.bias_B >= 330) FunctionGenerator.bias_B = 330;
					else {
						switch(buttonPressCounter) {
							case 0: FunctionGenerator.bias_B++; break;
							case 1: FunctionGenerator.bias_B += 10; break;
							case 2: FunctionGenerator.bias_B += 100; break;
							default: break;
						}
					}
					
					if (FunctionGenerator.bias_B_sign == POSITIVE) FG_SetBiasDC(FG1, 0, NEGATIVE);
					else FG_SetBiasDC(FG1, 0, POSITIVE);
					FG_SetBiasDC(FG1, (uint16_t)(abs(FunctionGenerator.bias_B) * 12.4), FunctionGenerator.bias_B_sign);
					
				}
							
				else if (encoderState == CCW) {
					display.stateChanged = true;
					if (FunctionGenerator.bias_B < 0) FunctionGenerator.bias_B_sign = NEGATIVE;
					else FunctionGenerator.bias_B_sign = POSITIVE;
					
						if (FunctionGenerator.bias_B <= -330) FunctionGenerator.bias_B = -330;
						else {
							switch(buttonPressCounter) {
								case 0: FunctionGenerator.bias_B--; break;
								case 1: FunctionGenerator.bias_B -= 10; break;
								case 2: FunctionGenerator.bias_B -= 100; break;
							}
						}

					if (FunctionGenerator.bias_B_sign == POSITIVE) FG_SetBiasDC(FG1, 0, NEGATIVE);
					else FG_SetBiasDC(FG1, 0, POSITIVE);
					FG_SetBiasDC(FG1, (uint16_t)(abs(FunctionGenerator.bias_B) * 12.4), FunctionGenerator.bias_B_sign);
				}
						
				else if (switchState) {
					if (buttonPressCounter < 2) buttonPressCounter++;
					else {
						buttonPressCounter = 0;
						mainDeviceState = PRIMARY_SCREENS;
						displayPointer = PTR_NULL;
					}
					display.stateChanged = true;
				}
				break;
				
				case PTR_BACK:
				mainDeviceState = PRIMARY_SCREENS;
				displayPointer = PTR_NULL;
				display.stateChanged = true;
				display.mainScreen = PARAMS_SCREEN;
				break;
				
				default: break;			}
		}
		else if (PARAMETER_LCD_POINTER_ON) {
			switch(displayPointer) {
				case PTR_BRIGHT:
				if (encoderState == CW) {
					display.stateChanged = true;
					lcdFunctionChanged = true;
					if (LCD.brightness >= 100) LCD.brightness = 100;
					else LCD.brightness++;
				}
				else if (encoderState == CCW) {
					display.stateChanged = true;
					lcdFunctionChanged = true;
					if (LCD.brightness <= 0) LCD.brightness = 0;
					else LCD.brightness--;
				}
				
				else if (switchState) {
					mainDeviceState = PRIMARY_SCREENS;
					displayPointer = PTR_NULL;
					display.stateChanged = true;
				}
				break;
				
				case PTR_CONTR:
				if (encoderState == CW) {
					display.stateChanged = true;
					lcdFunctionChanged = true;
					if (LCD.contrast >= 100) LCD.contrast = 100;
					else LCD.contrast++;
				}
				else if (encoderState == CCW) {
					display.stateChanged = true;
					lcdFunctionChanged = true;
					if (LCD.contrast <= 0) LCD.contrast = 0;
					else LCD.contrast--;
				}
				
				else if (switchState) {
					mainDeviceState = PRIMARY_SCREENS;
					displayPointer = PTR_NULL;
					display.stateChanged = true;
				}
				break;
				
				case PTR_BACK:
				mainDeviceState = PRIMARY_SCREENS;
				displayPointer = PTR_NULL;
				display.stateChanged = true;
				display.mainScreen = PARAMS_SCREEN;
				break;
				
				default: break;
			}
		}
	}
}