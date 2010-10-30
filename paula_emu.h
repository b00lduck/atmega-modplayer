#include <avr/io.h>

struct t_paulachannel {

	uint16_t 	addr;
	uint32_t	length;
	uint32_t 	finalrate;
	uint8_t 	volume;

	uint32_t 	loop_start;
	uint32_t 	loop_end;
	uint32_t 	loop_length;
	uint8_t		loop_enable;

	uint32_t	position;

	uint8_t		playing;

};

extern struct t_paulachannel paulachannel[4];

void paula_render();
void paula_init();
