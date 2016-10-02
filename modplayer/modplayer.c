#include <avr/interrupt.h>
#include "modplayer_config.h"
#include "modplayer.h"
#include "modplayer_paula_emu.h"
#include "../uart.h"

#include "modplayer_arptable.h"

struct t_songinfo songinfo;

uint8_t speed;

uint8_t songpos = 0;
uint8_t activerow = 0;
uint8_t activeframe = 0;

struct t_channelstate channelstate[4];
struct t_sampleheader sampleheader[31];

void trigger(uint8_t ch, struct t_paulachannel* pch, uint8_t sample_id, uint16_t rate) {

	channelstate[ch].sample_id = sample_id;
	channelstate[ch].FParp[0] = FParptable[0];
	channelstate[ch].FParp[1] = FParptable[0];
	channelstate[ch].FParp[2] = FParptable[0];
	channelstate[ch].volume_slide = 0;
	channelstate[ch].pitch_slide = 0;

	pch->addr = songinfo.offset_samples + sampleheader[sample_id].data_offset;
	pch->length = sampleheader[sample_id].length;

	pch->volume = sampleheader[sample_id].volume;

	pch->loop_start = sampleheader[sample_id].loop_start;
	pch->loop_end = sampleheader[sample_id].loop_end;
	pch->loop_length = pch->loop_end - pch->loop_start;
	pch->loop_enable = sampleheader[sample_id].loop;

	pch->position = 0;
	pch->playing = 1;
	
	if (rate < HALF_FREQ_THRESH) {
		channelstate[ch].rate_before_fx = rate << 1;
		pch->finalrate = rate << 1;
		pch->next_sample_in = rate << 1;
		pch->step = 2;
	} else {
		channelstate[ch].rate_before_fx = rate;
		pch->finalrate = rate;
		pch->next_sample_in = rate;
		pch->step = 1;
	}

}

/**
 *
 */
static inline void process_row() {
	uint8_t ch;

	uint32_t pre_offset = songinfo.offset_patterns + (songinfo.patterntable[songpos] << 10) + (activerow << 4);

	for (ch=0;ch<4;ch++) {

		struct t_paulachannel* pch;

		switch(ch) {
			case 0: pch = &paulachannel0; break;
			case 1: pch = &paulachannel1; break;
			case 2: pch = &paulachannel2; break;
			case 3: pch = &paulachannel3; break;
		}

		// Decode one division
		uint32_t offset = pre_offset + (ch << 2);

		uint8_t byte1 = READ_SONGDATA_BYTE(offset);
		uint8_t byte2 = READ_SONGDATA_BYTE(offset+1);
		uint8_t byte3 = READ_SONGDATA_BYTE(offset+2);
		uint8_t byte4 = READ_SONGDATA_BYTE(offset+3);
		uint16_t rate = ((byte1 & 0x0f) << 8) + byte2;
		uint8_t sample_id = (byte1 & 0xf0) + (byte3 >> 4);
		uint8_t effectdata = byte4;
		uint8_t effectid = byte3 & 0x0f;
		uint8_t effect_x = effectdata >> 4;
		uint8_t effect_y = effectdata & 0xf;
		
	/*	if (ch == playing_chan) {	

			uart_putc_hex(byte1);
			uart_puts_p(PSTR(" "));
			uart_putc_hex(byte2);
			uart_puts_p(PSTR(" "));
			uart_putc_hex(byte3);
			uart_puts_p(PSTR(" "));
			uart_putc_hex(byte4);
			uart_puts_p(PSTR("   "));
		}*/

		if (sample_id > 0) {
			trigger(ch,pch,sample_id - 1,rate & 0x0fff);
		}

		switch(effectid) {

			case 0x0: // arpeggio
				channelstate[ch].FParp[0] = FParptable[0];
				channelstate[ch].FParp[1] = FParptable[effect_x];
				channelstate[ch].FParp[2] = FParptable[effect_y];								
				break;

			case 0xc: // set volume
				//sampleheader[sample_id].volume = effectdata;
				pch->volume = effectdata;
				break;

			case 0xa: // volume slide
				if (effect_x > 0) {
				//	uart_puts_p(PSTR("volume slide up "));
					channelstate[ch].volume_slide = effect_x;
				} else if (effect_y > 0) {
				//	uart_puts_p(PSTR("volume slide down "));
					channelstate[ch].volume_slide = -effect_y;
				}
				break;

			case 0x1: // pitch slide
				channelstate[ch].pitch_slide = effectdata;
				break;

			case 0x2: // pitch slide
				channelstate[ch].pitch_slide = -effectdata;
				break;

			case 0x3: // pitch slide (NOT SUPPORTED YET)
				break;

			case 0x4: // vibrato (NOT SUPPORTED YET)
				break;

			case 0x6: // Continue 'Vibrato', but also do Volume slide (NOT SUPPORTED YET)
				break;

			case 0xb: // jump to pattern
				songpos = effectdata;
				activerow = 0;
				break;

			case 0xd: // end current pattern
				songpos++;
				activerow = 0;
				break;

			case 0xf: // set speed
				speed = effectdata & 0xf;
				break;

			case 0xe: // filter
				break;

			default:
				//uart_puts_p(PSTR("unhandled effect: ")); uart_putc_hex(effectid); uart_putc('\n');
				break;
	
		}
				
	}	
//	uart_puts_p(PSTR("\n"));

}

/**
 *
 */
static inline void next_row() {
	// advance to next row
	activerow++;

	if (activerow > 63) {
		activerow = 0;
		songpos++;
		if(songpos >= songinfo.numsongpos) songpos = 0;
	}

}

uint8_t arpindex = 0; 

/**
 *
 */
static inline void process_frame() {
	
	for (uint8_t ch=0;ch<4;ch++) {

	
		struct t_paulachannel* pch;

		switch(ch) {
			case 0: pch = &paulachannel0; break;
			case 1: pch = &paulachannel1; break;
			case 2: pch = &paulachannel2; break;
			case 3: pch = &paulachannel3; break;
		}

		if (pch->playing) {	
						
			// pitch slide
			if (channelstate[ch].pitch_slide != 0) {
				channelstate[ch].rate_before_fx += channelstate[ch].pitch_slide;
				if (channelstate[ch].rate_before_fx > 856) channelstate[ch].rate_before_fx = 856;
				else if (channelstate[ch].rate_before_fx < 113) channelstate[ch].rate_before_fx = 113;				
			}

			// arpeggio
			uint16_t arped_rate = (uint16_t)channelstate[ch].rate_before_fx * channelstate[ch].FParp[arpindex];			
			pch->finalrate = arped_rate >> 6;		

			// volume slide
			if (channelstate[ch].volume_slide != 0) {
				pch->volume += channelstate[ch].volume_slide;
				if (pch->volume < 0) pch->volume = 0;
				else if (pch->volume > 64) pch->volume = 64;				
			}
		
		}
		 
	}

	arpindex++;
	if (arpindex > 2) arpindex = 0;

	// do it
	//OCR1A = paulachannel[playing_chan].finalrate;

}

uint8_t c=0;

/**
 *
 */
ISR (TIMER2_COMPA_vect) {	
	c ^= 1; // every 2nd call only
	
	if(c) {

		if (activeframe == 0) {
			process_row();
			next_row();
		}
	
		process_frame();

		activeframe++;

		if (activeframe >= speed) {
			activeframe = 0;
		}
	
	}

}

/**
 *
 */
void copy_sampleinfo() {

	// look for M.K.
	if((READ_SONGDATA_BYTE(OFFSET_MK) == 'M') &&
		(READ_SONGDATA_BYTE(OFFSET_MK+1) == '.') &&
		(READ_SONGDATA_BYTE(OFFSET_MK+2) == 'K') &&
		(READ_SONGDATA_BYTE(OFFSET_MK+3) == '.')) {
		songinfo.numsamples = 31;
	} else {
		songinfo.numsamples = 15;
	}

	uint32_t soff=0;
	uint8_t c;

	// read SAMPLEINFO from FLASH into RAM
	for(c = 0; c < songinfo.numsamples; c++) {

		sampleheader[c].data_offset = soff + 4;
		uint32_t offset;
		offset = OFFSET_SAMPLE_HEADER + (SAMPLE_HEADER_LENGTH * c);	
		
		sampleheader[c].length = (uint8_t)READ_SONGDATA_BYTE(offset + 22) << 9;
		sampleheader[c].length += (uint8_t)READ_SONGDATA_BYTE(offset + 23) << 1;

		sampleheader[c].finetune = (int8_t)READ_SONGDATA_BYTE(offset + 24);
		sampleheader[c].volume = (uint8_t)READ_SONGDATA_BYTE(offset + 25);

		sampleheader[c].loop_start = (uint8_t)READ_SONGDATA_BYTE(offset + 26) << 9;
		sampleheader[c].loop_start += (uint8_t)READ_SONGDATA_BYTE(offset + 27) << 1;

		sampleheader[c].loop_length = (uint8_t)READ_SONGDATA_BYTE(offset + 28) << 9;
		sampleheader[c].loop_length += (uint8_t)READ_SONGDATA_BYTE(offset + 29) << 1;		

		sampleheader[c].loop_end = sampleheader[c].loop_start + sampleheader[c].loop_length;
		sampleheader[c].loop = (sampleheader[c].loop_length > 2);

		soff += sampleheader[c].length;

	}

}

/**
 *
 */
uint8_t get_highest_pattern_number() {
	uint8_t max = 0;
	uint32_t offset;
	offset = songinfo.offset_songheader + 2;

	uint8_t i;
	for(i=0;i<128;i++) {
		uint8_t tmp = READ_SONGDATA_BYTE(offset + i);	
		if (tmp > max) max = tmp;
	}

	return max;	
}

/**
 *
 */
void copy_songinfo() {

	songinfo.offset_songheader = OFFSET_SAMPLE_HEADER + SAMPLE_HEADER_LENGTH * songinfo.numsamples;
	songinfo.offset_patterns = songinfo.offset_songheader + 1 + 1 + 128 + 4;

	uint8_t i;
	
	for(i=0;i<20;i++) {
		songinfo.name[i] = (char)READ_SONGDATA_BYTE(i);
	}

	songinfo.numpatterns = get_highest_pattern_number() + 1;
	songinfo.numsongpos = READ_SONGDATA_BYTE(songinfo.offset_songheader);	
	
	for(i=0;i<128;i++) {
		songinfo.patterntable[i] = READ_SONGDATA_BYTE(songinfo.offset_songheader + 2 + i);
	}

}

/**
 *
 */
void modplayer_init() {	
	
	//Set TIMER2 (Frames)
	TIMSK2 &= ~(1 << OCIE2A);
	TCCR2A = (1<<WGM21);
	TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20);
	OCR2A = 200;

	speed = 6;
	copy_sampleinfo();
	copy_songinfo();

	songinfo.offset_samples = songinfo.offset_patterns + ((uint32_t)songinfo.numpatterns * 1024);		

}

/**
 *
 */
void get_sample_name(char* buf, uint8_t id) {
	uint16_t offset = OFFSET_SAMPLE_HEADER + SAMPLE_HEADER_LENGTH*id;
	for(uint8_t i=0; i<22; i++) {
		buf[i] = READ_SONGDATA_BYTE(offset+i);			
	}	
}

/**
 *
 */
void modplayer_play() {
	TIMSK2 |= (1 << OCIE2A);	
	TIMSK1 |= (1 << OCIE1A);
}

/**
 *
 */
void modplayer_stop() {
	TIMSK2 &= ~(1 << OCIE2A);
	TIMSK1 &= ~(1 << OCIE1A);
	songpos = 0;
	activerow = 0;
	activeframe = 0;
}
