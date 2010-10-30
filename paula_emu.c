#include "global.h"
#include "paula_emu.h"
#include "moduledata.h"

struct t_paula_channel paula_channel[4];

uint32_t FP16_ratefactor;

void init_paula() {
	// beware of the overflow
	uint32_t a = ((uint32_t)REFERENCE_RATE << FPACBITS) / SAMPLERATE;
	FP16_ratefactor = a * 428;

}
int8_t paula_render() {

	int16_t sum = 0;

	uint8_t ch;

	for (ch=0;ch<4;ch++) {

		if (paula_channel[ch].playing) {

			// LOOP
			if (paula_channel[ch].loop_enable) {			
				if (paula_channel[ch].FP16_position > paula_channel[ch].FP16_loop_end) {
					paula_channel[ch].FP16_position -= (paula_channel[ch].FP16_loop_length);
				}
			} 

			// PLAY
			if (paula_channel[ch].FP16_position < paula_channel[ch].FP16_length) {								
	
				int8_t tmp = (int8_t)pgm_read_byte(&moduledata[paula_channel[ch].addr + (paula_channel[ch].FP16_position >> FPACBITS)]);

				sum += (tmp * paula_channel[ch].volume) / 128;

				paula_channel[ch].FP16_position += paula_channel[ch].FP16_finalrate;

			} else {

				paula_channel[ch].playing = 0;

			}		

		}

	}	
	

	return (sum / 5);
}
