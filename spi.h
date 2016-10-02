#pragma once

void spi_init();

static inline void spi_transmit(uint8_t cData) {
	SPDR = cData;
	while(!(SPSR & (1<<SPIF)));
}
