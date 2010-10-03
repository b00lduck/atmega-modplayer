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
					paula_channel[ch].FP16_position = paula_channel[ch].FP16_loop_start;
				}
			} 

			// PLAY
			if (paula_channel[ch].FP16_position < paula_channel[ch].FP16_length) {								
	
				/*
				uint16_t current_position = (channelstate[ch].FPsamplepos >> FPACBITS);		
								
				// linear interpolation 8:8 fixedpoint
				uint8_t fraction = (channelstate[ch].FPsamplepos & FPA_FRACMASK) >> 8;
				int16_t a = GET_SBYTE(channelstate[ch].sample_id, current_position);
				int16_t b = GET_SBYTE(channelstate[ch].sample_id, current_position + 1);

				int8_t tmp = (a * (255-fraction) + (b * fraction));
				
				sum += tmp;

				channelstate[ch].FPsamplepos += channelstate[ch].FPrate;
				*/

				sum += (int8_t)pgm_read_byte(&moduledata[paula_channel[ch].addr + (paula_channel[ch].FP16_position >> FPACBITS)]);				

				paula_channel[ch].FP16_position += FP16_ratefactor / paula_channel[ch].rate;

			} else {

				paula_channel[ch].playing = 0;

			}		

		}

	}	
	

	return (sum / 4);
}
