#include <avr/io.h>

struct t_paula_channel {

	uint16_t 	addr;
	uint32_t	FP16_length;
	uint16_t 	rate;
	uint8_t 	volume;

	uint32_t 	FP16_loop_start;
	uint32_t 	FP16_loop_end;
	uint8_t		loop_enable;

	uint32_t	FP16_position;

	uint8_t		playing;

} ;

extern struct t_paula_channel paula_channel[4];

int8_t paula_render();
void init_paula();
