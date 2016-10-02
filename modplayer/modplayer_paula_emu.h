#pragma once
#include <avr/io.h>

struct t_paulachannel {

	uint32_t 	addr;
	uint32_t	length;
	uint16_t 	finalrate;
	int8_t 		volume;

	uint32_t 	loop_start;
	uint32_t 	loop_end;
	uint32_t 	loop_length;
	uint8_t		loop_enable;

	uint32_t	position;

	uint8_t		playing;

	int16_t		next_sample_in;

	int16_t		out;

	uint8_t		step;

};

extern struct t_paulachannel paulachannel0;
extern struct t_paulachannel paulachannel1;
extern struct t_paulachannel paulachannel2;
extern struct t_paulachannel paulachannel3;

void paula_init();

#define LOWEST_STEP 150
