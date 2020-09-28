/*
 * SPIMaster.h
 *
 * Created: 1/5/2019 7:33:26 PM
 *  Author: MX
 */ 


#ifndef SPIMASTER_H_
#define SPIMASTER_H_



 /* MCP42100 */ 
#define CONTRAST_ADDR 0b00010001
/* MCP4922 */
#define VOLUME_ADDR  0b01110000 // DACA_A
#define AMPLITUDE_A_ADDR 0b11110000 // DACA_B
#define AMPLITUDE_B_ADDR 0b11110000 // DACB_B
#define BIAS_A_POS_ADDR		 0b11110000	   // DACA_BIAS
#define BIAS_A_NEG_ADDR		 0b01110000	   // DACA_BIAS
#define BIAS_B_POS_ADDR		 0b11110000	   // DACB_BIAS
#define BIAS_B_NEG_ADDR		 0b01110000	   // DACB_BIAS
#define AD9834_CONSECUTIVE_WRITE 0x21
#define AD9834_FREQUENCY_REGISTER_ADDR 0x40
#define AD9834_PHASE_REGISTER_ADDR 0xC0
#define AD9834_EXIT_RESET 0x20

/* AD9834 */
#define DB15	(1 << 15)
#define DB14	(1 << 14)
#define B28		(1 << 13)
#define HLB		(1 << 12)
#define FSEL	(1 << 11)
#define PSEL	(1 << 10)
#define PIN_SW	(1 << 9)
#define RESET	(1 << 8)
#define SLEEP1	(1 << 7)
#define SLEEP12 (1 << 6)
#define OPBITEN (1 << 5)
#define SIGN	(1 << 4)
#define DIV2	(1 << 3)
#define MODE	(1 << 1)

/* MCP4922 */
#define nA_B	(1 << 15)
#define BUF		(1 << 14)
#define nGA 	(1 << 13)
#define nSHDN	(1 << 12)

//PROTOTYPES
void SPI_Write16Bit(uint8_t address_in, uint8_t data_in, enum Device device);
void SPI_InitAD9834(void);
void SPI_InitAll(void);

#endif /* SPIMASTER_H_ */