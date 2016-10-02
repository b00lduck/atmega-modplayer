#include "stdinc.h"
#include "tools.h"
#include "spi.h"


void spi_init() {
	/* Set MOSI and SCK output, all others input */
	DDRB |= (1<<5)|(1<<7);
	
    /* switch to highest SPI frequency possible */
    SPCR &= ~((1 << SPR1) | (1 << SPR0)); /* Clock Frequency: f_OSC / 4 */
    SPSR |= (1 << SPI2X); /* Doubled Clock Frequency: f_OSC / 2 */
}



