#ifndef MAIN_H_
#define MAIN_H_

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

//#define PRE_PROG

#define POSITIVE false
#define NEGATIVE true

/* Constants */
#define VDD	  3.3
#define RES_0 820
#define RES_1 470
#define MAX_8BIT 255
#define MAX_10BIT 1023
#define MAX_12BIT 4095
#define DAC_REF_RATIO 3405
#define DAC_DC_RATIO  40.95
#define AD9834_FREQ_FACTOR 10.737418
#define AD9834_CLOCK_FREQUENCY 25000000UL
#define VBAT_DIVIDER_RATIO 1.696
#define EEPROM_ADRESS_SPAN 128
#define LCD_LINE_LENGTH 20
#define POWER_ADC_THRESHOLD 420
#define BATTERY_CMD_LENGTH 10
#define POWER_CMD_LENGTH 8
#define DELAY_COMMAND_MS 1000
#define DELAY_DATA_MS 10

/* SPI Chip Enable  */
#define SPI_CE_DDR		DDRC
#define LCD_POT_DDR		DDRA
#define SPI_CE_PORT		PORTC
#define LCD_POT_PORT	PORTA
#define DACA_NCE		(1 << PINC2)
#define DACB_NCE		(1 << PINC3)
#define DACA_BIAS_NCE	(1 << PINC4)
#define DACB_BIAS_NCE	(1 << PINC5)
#define FG0_NCE			(1 << PINC6)
#define FG1_NCE			(1 << PINC7)
#define POT_LCD_NCE		(1 << PINA4)

#define PIN_ENCODER PINB
#define ENCODER_PORT PORTB
#define ENCODER_DDR DDRB
#define ENCODER_B (1 << PINB2)
#define ENCODER_A (1 << PINB3)

/* LCD Definitions */
#define LCD_CONTROL_DDR		DDRB
#define LCD_CONTROL_PORT	PORTB
#define LCD_DATA_DDR		DDRD
#define LCD_DATA_PORT		PORTD
#define LCD_RS				(1 << PINB0)
#define LCD_E				(1 << PINB1)
#define LCD_D4				(1 << PIND4)
#define LCD_D5				(1 << PIND5)
#define LCD_D6				(1 << PIND6)
#define LCD_D7				(1 << PIND7)

/* ADC Converter Inputs */
#define ADC_DDR		DDRA
#define ADC_PIN		PINA
#define VBAT_ADC	(1 << PINA0)
#define PWR_IND		(1 << PINA1)

/* SPI Bus */
#define SPI_DDR		DDRB
#define SPI_PORT	PORTB
#define SPI_PIN		PINB
#define SPI_MOSI	(1 << PINB5)
#define SPI_MISO	(1 << PINB6)
#define SPI_SCK		(1 << PINB7)

/* UART */
#define UART_DDR	DDRD
#define UART_PORT	PORTD
#define UART_PIN	PIND
#define UART_RX		(1 << PIND0)
#define UART_TX		(1 << PIND1)

/* Function Generator Control Pins */
#define FG_SEL_DDR	DDRC
#define FG_SEL_PORT PORTC
#define FG0_SEL (1 << PINC0)
#define FG1_SEL (1 << PINC1)

/* Push Button */
#define PB_DDR		DDRD
#define PB_PIN		PIND
#define S_INT (1 << PIND2)

/* Misc */
#define MISC_DDR	DDRA
#define MISC_PORT PORTA
#define PS_HOLD (1 << PINA2)
#define BUZZER	(1 << PINA3)

/* Macro Functions */
#define ENABLE_DEVICE()		MISC_PORT |= PS_HOLD	
#define DISABLE_DEVICE()	MISC_PORT &= ~PS_HOLD	
#define BUZZER_H()		MISC_PORT |= BUZZER	
#define BUZZER_L()	MISC_PORT &= ~BUZZER

//#define ENABLE_TIMER()  0 // TCCR1B |= (1 << CS10) | (1 << CS12)
//#define DISABLE_TIMER() 0 //TCCR1B &= ~(1 << CS10) & ~(1 << CS12)

#define RESET_DEVICE()        \
do                          \
{                           \
	wdt_enable(WDTO_15MS);  \
	for(;;)                 \
	{                       \
	}                       \
} while(0)

/* LCD Displays */ 
#define LCD_MAIN_STRINGA_1			  " Channel:A, Type:OFF"
#define LCD_MAIN_STRINGB_1			  " Channel:B, Type:OFF"
#define LCD_MAIN_STRING_2			  " Amplitude: 0.00[V] "
#define LCD_MAIN_STRING_3			  " Freq: 0.000.000[Hz]"
#define LCD_MAIN_STRING_4			  " Bias: +0.00[V]     "

#define LCD_MAIN_SETTINGS_STRING_1	  "  V(Battery):X.XX[V]"
#define LCD_MAIN_SETTINGS_STRING_2	  "  USB Charging: XXX "
#define LCD_MAIN_SETTINGS_STRING_3    "  Settings          "
#define LCD_MAIN_SETTINGS_STRING_4	  "  Shutdown          "

#define LCD_PROFILE_SETTINGS_STRING_1 " <Profile Settings> "
#define LCD_PROFILE_SETTINGS_STRING_2 "  Save Profile      "
#define LCD_PROFILE_SETTINGS_STRING_3 "  Load Profile      "
#define LCD_PROFILE_SETTINGS_STRING_4 "       <BACK>       "

#define LCD_SCREEN_SETTINGS_STRING_1  "  <<LCD Settings>>  "
#define LCD_SCREEN_SETTINGS_STRING_2  "  Brightness:100[%] "
#define LCD_SCREEN_SETTINGS_STRING_3  "  Contrast:  100[%] "
#define LCD_SCREEN_SETTINGS_STRING_4  "                    "

#define LCD_SHUTDOWN_STRING_1		  "  Perform Reset     "
#define LCD_SHUTDOWN_STRING_2		  "  Power Off         "
#define LCD_SHUTDOWN_STRING_3		  "  Factory Settings  "
#define LCD_SHUTDOWN_STRING_4		  "       <BACK>       "

bool g_bSPITransferCompleted;
typedef enum { PRIMARY_SCREENS,  MENU_POINTER_ON, PARAMETER_POINTER_ON, PARAMETER_LCD_POINTER_ON} MainDeviceState;
typedef enum EncoderStates {NONE, CW, CCW} EncoderState;
typedef enum MainScreens { MAIN_SCREEN_A, MAIN_SCREEN_B, PARAMS_SCREEN, SETTINGS_SCREEN, PROFILE_SCREEN, LCD_SCREEN, SHUTDOWN_SCREEN } MainScreen;
typedef enum DisplayPointers { PTR_NULL, PTR_BACK,
	PTR_TYPE_A, PTR_FREQ_A, PTR_BIAS_A, PTR_AMP_A, PTR_TYPE_B, PTR_FREQ_B, PTR_BIAS_B, PTR_AMP_B,
	PTR_SETT,  PTR_SHUTDOWN,
	PTR_SAVE_PROF, PTR_LOAD_PROF,
	PTR_BRIGHT, PTR_CONTR,
} DisplayPointer;

enum Device { DACA = 1, DACB, DACA_BIAS, DACB_BIAS, FG0, FG1, LCD_POT };
enum WaveformType { SINE = 1, TRIANGLE, SQUARE, DC, OFF };
enum LEDState { RED = 1, GREEN, BLUE, LED_OFF };

struct {
	volatile bool previousA;
	volatile bool previousB;
	uint8_t encoderSeqCntCW;
	uint8_t encoderSeqCntCCW;
} Encoder;

typedef struct {
	uint32_t frequency_A, frequency_B;
	uint16_t amplitude_A, amplitude_B;
	enum WaveformType output_type_A, output_type_B;
	int16_t bias_A, bias_B;
	bool bias_A_sign, bias_B_sign;
} FGX;

struct {
	uint16_t battery_voltage;
	bool   ac_power_PowerStatus;
} PowerStatus;


struct LCD_PARAMETERS {
	uint8_t brightness;
	uint8_t contrast;
	} LCD;

struct UI_STRINGS {
	char frequency_A[7], frequency_B[7];
	char amplitude_A[2], amplitude_B[2];
	char bias_A[3], bias_B[3];
	char type_A[3], type_B[3];
	char bias_A_sign, bias_B_sign;
	char batteryPowerStatus[3];
	char lcd_brightness[3];
	char lcd_contrast[3];
} UI;

typedef struct {
	bool stateChanged;
	MainScreen mainScreen;
} Screen;

void Buzzer_Beep();
void Buzzer_PlayMelody(bool power_on);

EncoderState Main_PollEncoder();
bool Main_PollSwitch();
void Main_ShutdownDevice(bool is_erase_requested);
void Main_UintToString(uint32_t number, char *string, uint8_t length);

void Init_Timer();
void Init_Ports();
void Init_Device();
void Init_ADC();
void Init_UI();
void Init_ClearWaveformValues();
void Init_ClearUIValues();
void Init_ClearLCDParameterValues();

void FG_SelectOutputType(enum Device device, enum WaveformType waveformType);
void FG_SetFunction(enum Device device, uint32_t waveFrequency, enum WaveformType mode);
int FG_SetAmplitude(uint16_t valueIn, enum Device device);
int FG_SetBiasDC(enum Device device, uint16_t value, bool sign);

void LCD_BrightnessAnimation();
void LCD_SetContrast(uint8_t value);
int LCD_SetBrightness(uint8_t value);
uint8_t LCD_GetParameters(uint8_t parameter);

void EEPROM_Erase_1K();
void EEPROM_SaveProfile();
void EEPROM_LoadProfile();

void Power_UpdateBatteryStatus();
void Power_UpdateAcStatus();

void Handle_LCD(MainScreen screen, DisplayPointer displayPointer, MainDeviceState mainDeviceState, FGX FunctionGenerator);
void Handle_LCDParameter(DisplayPointer displayPointer);

#endif /* MAIN_H_ */