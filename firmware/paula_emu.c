#include "global.h"
#include "paula_emu.h"
#include "moduledata.h"

struct t_paulachannel paulachannel[4];

void paula_init() {

}

void paula_render() {
	
	uint8_t ch = TESTCHAN;
	uint8_t out;;

	if (paulachannel[ch].playing) {

		paulachannel[ch].position++;

		// LOOP
		if (paulachannel[ch].loop_enable) {			
			if (paulachannel[ch].position >= paulachannel[ch].loop_end) {
				paulachannel[ch].position -= (paulachannel[ch].loop_length);
			}
		} else if (paulachannel[ch].position >= paulachannel[ch].length) {
			paulachannel[ch].playing = 0;
		}
		
		int16_t tmp = (int8_t)pgm_read_byte(&moduledata[paulachannel[ch].addr + paulachannel[ch].position]);

		tmp *= paulachannel[ch].volume;

		tmp /= 256;

		out = tmp + 127;
	
	}
	
	OCR0 = out;

	return;
	/*

	int16_t sum = 0;

	uint8_t ch;

	for (ch=0;ch<4;ch++) {

		if (paulachannel[ch].playing) {

			// LOOP
			if (paulachannel[ch].loop_enable) {			
				if (paulachannel[ch].FP16_position > paulachannel[ch].FP16_loop_end) {
					paulachannel[ch].FP16_position -= (paulachannel[ch].FP16_loop_length);
				}
			} 

			// PLAY
			if (paulachannel[ch].FP16_position < paulachannel[ch].FP16_length) {								
	
				int8_t tmp = (int8_t)pgm_read_byte(&moduledata[paulachannel[ch].addr + (paulachannel[ch].FP16_position >> FPACBITS)]);

				sum += (tmp * paulachannel[ch].volume) / 128;

				paulachannel[ch].FP16_position += paulachannel[ch].FP16_finalrate;

			} else {

				paulachannel[ch].playing = 0;

			}		

		}

	}
	*/	
	

}
