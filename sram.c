#include "stdinc.h"
#include "tools.h"
#include "spi.h"
#include "sram.h"


void sram_init() {	

	DDRD  |= 0b11111000;

	// 
	CBI(PORTD,6);	
	CBI(PORTD,7);

	// Disable latches (active high), Disable ram control lines (active low)
	PORTD = 0b00111000;

	// Address bytes 16-18
	DDRB |= 0b00000111;

}


void sram_write(uint32_t addr, uint8_t data) {

	sram_set_address(addr);

	// set data
	PORTA = data;

	// Chip Select SRAM
	// Write Enable SRAM
	CBI(PORTD,3);
	CBI(PORTD,5);

	// Write Un-Enable SRAM
	// Chip Un-Select SRAM
	SBI(PORTD,5);
	SBI(PORTD,3);
	
}

void sram_test() {
	uint32_t i;
	for(i=0; i<MEMORY_SIZE; i++) {
		sram_write(i,i);	
	}
}

void sram_clear() {
	uint32_t i;
	for(i=0; i<MEMORY_SIZE; i++) {
		sram_write(i,0);
	}
}

