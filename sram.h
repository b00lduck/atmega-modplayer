#include <avr/io.h>
#include "tools.h"
#include "spi.h"

//efine MEMORY_SIZE 524288
#define MEMORY_SIZE 512

void sram_init();
void sram_clear();
void sram_write(uint32_t addr, uint8_t data);

static inline void sram_set_address(uint32_t addr) {

	DDRA = 0xff;

	PORTB &= 0b11111000;
	PORTB |= ((addr >> 16) & 0b111);

	SBI(PORTD,6);
	PORTA = addr >> 8;
	CBI(PORTD,6);

	SBI(PORTD,7);
	PORTA = addr;
	CBI(PORTD,7);	
}


static inline uint8_t sram_read(uint32_t addr) {

	sram_set_address(addr);

	DDRA = 0x00;

	// Chip Select SRAM
	// Output Enable SRAM
	CBI(PORTD,3);
	CBI(PORTD,4);
	
	NOP

	// Chip Un-Select SRAM
	SBI(PORTD,3);

	// read data
	uint8_t ret = PINA;	

	// Output Un-Enable SRAM
	SBI(PORTD,4);

	return ret;

}


