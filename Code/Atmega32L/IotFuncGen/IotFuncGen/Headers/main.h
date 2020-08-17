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
#include "WifiParams.h"

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
/*-----------------------------*/
/*--  PORT I/O Definitions	 --*/

#define PRE_PROG 0
#define SKIP_LOGO 0
#define INITIATE_WLAN_PAIRING 0
#define INITIATE_DIRECT_PAIRING 0
#define PERFORM_FG_TEST 0
#define TEST_BUZZER 0
#define TEST_ADC 0
#define RGB_LED_SUPPORT 0

#define POSITIVE false
#define NEGATIVE true

#define VDD	  3.3
#define RES_0 820
#define RES_1 470
#define MAX_8BIT 255
#define MAX_10BIT 1023
#define MAX_12BIT 4095
#define MAX_AMPLITUDE 1.00
#define DAC_REF_RATIO 3405
#define DAC_DC_RATIO  40.95
#define AD9834_FREQ_FACTOR 10.737418
#define AD9834_CLOCK_FREQUENCY 25000000UL
#define VBAT_DIVIDER_RATIO 0.589
#define EEPROM_ADRESS_SPAN 128
#define LCD_LINE_LENGTH 20
#define BATTERY_ADC_FACTOR_A 0.977
#define BATTERY_ADC_FACTOR_B 5.602
#define POWER_ADC_THRESHOLD 500
#define BATTERY_CMD_LENGTH 10
#define POWER_CMD_LENGTH 8

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

enum DEVICES { DACA = 1, DACB, DACA_BIAS, DACB_BIAS, FG0, FG1, LCD_POT };
enum MODES { SINE = 1, TRIANGLE, SQUARE, DC, OFF };
enum LED_STATES { RED = 1, GREEN, BLUE, LED_OFF };
enum MENU_STATES_MAIN { INIT_STATE = 1, PROCEED_BUTTON_PERFORM, BUTTON_STATE_SELECTION, WIFI_LAN_PAIRING_INIT, DIRECT_PAIRING_INIT, IDLE, };
enum DIRECT_PAIRING_STATES {SHOW_DIRECT_MESSAGE = 1, ECHO_OFF_COMMAND, SET_AP_MODE, OPEN_DIRECT_AP, SET_MUX_COMMAND, OPEN_SOCKET_SERVER, WAIT_FOR_DEVICE, MESSAGE_HANDLING_STATE };		
enum MESSAGE_HANDLER_STATES { MESSAGE_HANDLER_IDLE = 1, PROVIDE_DETAILS, BOOT_FROM_APP, FUNCTION_HANDLER, LCD_PARAM_ADJUST, VOLUME_ADJUST, RESET_HANDLER };
enum LAN_PAIRING_STATES { SHOW_LAN_MESSAGE = 1, ECHO_OFF, SET_AP_MODE_LAN, OPEN_LOCAL_AP, SET_MUX_COMMAND_LAN, OPEN_LOCAL_SOCKET, WAIT_FOR_DEVICE_CONNECT, RETRIEVE_MAC, SEND_MAC, RETRIEVE_CREDENTIALS };
enum WLAN_COMMUNICATION_STATES {INIT_MESSAGE = 1, REPEAT_OFF, REQUEST_WIFI, CREATE_SOCKET_SERVER_WLAN, WAIT_FOR_WLAN_DEVICE, DEVICE_CONNECTED};
	
/* RGB LED  */
#define LED_DDR		DDRB
#define LED_PORT	PORTB
#define LED_B		(1 << PINB2)
#define LED_G		(1 << PINB3)
#define LED_R		(1 << PINB4)

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

#define ENABLE_DEVICE()		MISC_PORT |= PS_HOLD	// Enable interrupt on RX complete
#define DISABLE_DEVICE()	MISC_PORT &= ~PS_HOLD	// Disable RX interrupt
#define BUZZER_H()		MISC_PORT |= BUZZER	// Enable interrupt on RX complete
#define BUZZER_L()	MISC_PORT &= ~BUZZER	// Disable RX interrupt
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

#define ADC_V true
#define CMP_V false

#define PARAM_FREQ	'F'
#define PARAM_PHASE 'P'
#define PARAM_TYPE	'T'
#define PARAM_AMP	'A'
#define PARAM_BIAS	'B'

#define PARAM_CH0 '0'
#define PARAM_CH1 '1'

#define TYPE_SINE 'S'
#define TYPE_SQUARE 'Q'
#define TYPE_TRIANGLE 'T'
#define TYPE_DC	'D'
#define TYPE_OFF 'O'

#define FG_DATA_LENGTH 32
#define MAX_COMMAND_LENGTH 128
#define FG_DATA_START_NUM 9
#define NUMBER_OF_FREQUENCY_DIGITS 8
#define MAXIMUM_COMMAND_RETRIES 20
#define TIMER_PERIOD 1 // In SEC
#define TIMER_COMPARE_VALUE (F_CPU / 1024) * TIMER_PERIOD

#define FIRMWARE_VERSION_A '2'
#define FIRMWARE_VERSION_B '0'

#define BRIGHTNESS 0
#define CONTRAST 1
#define VOLUME 2

#define MAX_STRING_BUFFER 20
#define ZERO_AMPLITUDE "0.00"
#define ZERO_BIAS "+0.00"
#define START_WLAN_MSG			" Starting Wi-Fi LAN "
#define START_DIRECT_MSG		"  Starting Direct   "
#define START_COMM_MSG			"communication wizard"
#define ESP32_STATUS_MSG_OK		"ESP32 Device......OK"
#define ESP32_STATUS_MSG		"ESP32 Device......  "
#define REQUEST_NETWORK_MSG		"Opening Network...  "
#define REQUEST_NETWORK_MSG_OK	"Opening Network...OK"
#define CREATING_SERVER_MSG		"Creating Server...  "
#define CREATING_SERVER_MSG_OK	"Creating Server...OK"
#define WAIT_FOR_DEVICE_MSG		"<Waiting For Device>"
#define DEVICE_CONNECTED_MSG	" <Device connected> "
#define SHUTDOWN_MSG			"  Shutting Down...  "
#define IN_X_SEC_MSG			"      in X sec      "
#define REBOOT_MSG				"   Rebooting...     "
#define PB_MSG					"Push button to begin"

struct MAIN_STRUCTURE {
	uint32_t frequency_A, frequency_B;
	uint16_t phase_A, phase_B;
	uint16_t amplitude_A, amplitude_B;
	enum MODES output_type_A, output_type_B;
	uint16_t bias_A, bias_B;
	bool bias_A_sign, bias_B_sign;
} FUNCGEN;

struct STATUS_STRUCTURE {
	uint8_t battery_voltage;
	bool   ac_power_status;
	bool socket_active;
} STATUS;

struct WIFI_STRUCTURE {
	char SSID[MAX_WIFI_SSID_LENGTH];
	char PASS[MAX_WIFI_PASS_LENGTH];
	char device_MAC[MAC_STRING_LENGTH];
	char encryption;
} WIFI;

struct LCD_PARAMETERS {
	uint8_t brightness;
	uint8_t contrast;
	uint8_t volume;
	} LCD;

struct UI_STRINGS {
	char frequency_A[8], frequency_B[8];
	char amplitude_A[4], amplitude_B[4];
	char bias_A[5], bias_B[5];
	char type_A[3], type_B[3];
} UI;

uint8_t update_sg_param_value(uint8_t parameter);
void set_volume(uint8_t value);
void beep();
void shutdown_sequence(bool is_erase_requested);
void Init_Timer();
void Init_Ports();
void Init_Device();
void Init_ADC();
void Init_UI();
void set_output_selection(enum DEVICES device, enum MODES device_mode);
void set_functionality(enum DEVICES device, uint32_t device_freq, enum MODES mode);
int set_amplitude(uint16_t val_in, enum DEVICES device);
void set_LED(enum LED_STATES LED);
bool poll_switch();
void test_LEDS();
void set_animated_brightness();
void set_LCD_contrast(uint8_t value);
int set_LCD_brightness(uint8_t value);
int set_dc_bias (enum DEVICES device, uint16_t value, bool sign);
void erase_EEPROM_1K();
void wifi_lan_pairing();
void start_wlan_communication();
char * create_wifi_command();
void save_wifi_credentials();
void retrieve_wifi_credentials();
void direct_pairing();
void socket_message_handler();
void update_battery_status();
void update_ac_power_status();
char * get_battery_status();
char * get_ac_power_status();
uint8_t get_li_ion_percentage (uint16_t vin);
void clear_active_UI();
void update_UI_activity(uint8_t line, uint8_t segment);
void update_complete_UI();
uint32_t retrieve_frequency_uint32(char channel_in);
uint16_t retrieve_amplitude_12_bit(char channel_in);
uint16_t retrieve_bias_12_bit(char channel_in);
bool retrieve_bias_sign();
void set_parameter();
void clear_all_values();
void clear_funcgen_values();
void clear_status_values();
void clear_wifi_values();
void print_LCD_line_stored(char *msg, uint8_t line_in);
void init_UI_array();
#endif /* MAIN_H_ */