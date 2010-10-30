
#include "global.h"
#include "modplayer.h"
#include "moduledata.h"
#include "paula_emu.h"

uint32_t offset_samples;

uint8_t numpatterns;
uint8_t patterntable[128];
uint8_t numsongpos;

uint8_t speed = 6;

uint8_t songpos = 0;
uint8_t activerow = 0;
uint8_t activeframe = 0;

struct t_channelstate channelstate[4];
struct t_sampleheader sampleheader[31];

void trigger(uint8_t ch, uint8_t sample_id, uint16_t rate) {

	if (ch != TESTCHAN) return;

	channelstate[ch].sample_id = sample_id;
	channelstate[ch].rate_before_fx = rate;
	channelstate[ch].arp[0] = 0;
	channelstate[ch].arp[1] = 0;
	channelstate[ch].arp[2] = 0;

	paulachannel[ch].addr = offset_samples + sampleheader[sample_id].data_offset;
	paulachannel[ch].length = sampleheader[sample_id].length;
	paulachannel[ch].finalrate = rate;
	paulachannel[ch].volume = 64;

	paulachannel[ch].loop_start = sampleheader[sample_id].loop_start;
	paulachannel[ch].loop_end = sampleheader[sample_id].loop_end;
	paulachannel[ch].loop_length = paulachannel[ch].loop_end - paulachannel[ch].loop_start;
	paulachannel[ch].loop_enable = sampleheader[sample_id].loop;

	paulachannel[ch].position = 0;
	paulachannel[ch].playing = 1;

	OCR1A = rate;
	
}


/**
 *
 */
void next_row() {

	activerow++;

	if (activerow >= 63) {
		activerow = 0;
		songpos++;
		if(songpos >= numsongpos) songpos = 0;
	}

}

/**
 *
 */
void process_row() {
	uint8_t ch;

	uint32_t pre_offset = OFFSET_PATTERNS + (patterntable[songpos] << 10) + (activerow << 4);

	for (ch=0;ch<4;ch++) {

		// Decode one division
		uint32_t offset = pre_offset + (ch * 4);

		uint8_t byte1 = pgm_read_byte(&moduledata[offset]);
		uint8_t byte2 = pgm_read_byte(&moduledata[offset+1]);
		uint8_t byte3 = pgm_read_byte(&moduledata[offset+2]);
		uint8_t byte4 = pgm_read_byte(&moduledata[offset+3]);
		uint16_t rate = ((byte1 & 0x0f) << 8) + byte2;
		uint8_t sample_id = (byte1 & 0xf0) + (byte3 >> 4);
		uint8_t effectdata = byte4;
		uint8_t effectid = byte3 & 0x0f;	

		if (sample_id > 0) {
			trigger(ch,sample_id - 1,rate & 0x0fff);
		}

		switch(effectid) {

			case 0:
				channelstate[ch].arp[0] = 0;
				channelstate[ch].arp[1] = effectdata >> 4;
				channelstate[ch].arp[2] = effectdata & 0xf;								
				break;

			case 0xc:
				sampleheader[sample_id].volume = effectdata;
				paulachannel[ch].volume = effectdata;
				break;

			case 0xb:
				songpos = effectdata;
				break;

			case 0xf:
				speed = effectdata & 0xf;
				break;
		
		}
				
	}	

}

// 6bit FPA
//uint16_t FParptable[16] = {64,68,72,76,81,85,91,96,102,108,114,121,128};
uint16_t FParptable[16] = {51,54,57,61,65,68,72,77,81,86,91,97,102};

// 7bit FPA
//uint16_t FParptable[16] = {128,136,144,152,161,171,181,192,203,215,228,241,256,271,287,304};

// 8bit FPA
//uint16_t FParptable[16] = {256,271,287,304,322,342,362,383,406,430,456,483,512,542,574,608};

//10bit FPA
//uint16_t FParptable[16] = {1024,1085,1149,1218,1290,1366,1448,1534,1625,1721,1823,1932,2047,2168,2297,2433};

//16bit FPA
//uint32_t FParptable[16] = {65536,69429,73553,77922,82551,87454,92649,98152,103982,110159,116702,123634,130978,138758,147001,155733};
 
/**
 *
 */
void process_frame() {

	uint8_t arpindex = activeframe % 3;

	uint8_t ch;
	for (ch=0;ch<4;ch++) {

		if (ch == TESTCHAN) {

			if (paulachannel[ch].playing) {	
				
				uint8_t arptable_index = channelstate[ch].arp[arpindex];
				
				uint16_t arped_rate = channelstate[ch].rate_before_fx * (FParptable[arptable_index]);	
				
				paulachannel[ch].finalrate = arped_rate >> 6;
			
				OCR1A = paulachannel[ch].finalrate;
				//OCR1A = channelstate[ch].rate_before_fx;
			}
		 
		}

	}


}

/**
 *
 */
void modplayer_next_frame() {
	
	if (activeframe < speed) {
		process_frame();
		activeframe++;
	} else {
		activeframe = 0;
		next_row();
		process_row();
	}

}

/**
 *
 */
void copy_sampleinfo() {

	uint32_t soff=0;
	uint8_t c;

	// read SAMPLEINFO from FLASH into RAM
	for(c=0;c<31;c++) {

		sampleheader[c].data_offset = soff;
		uint32_t offset;
		offset = OFFSET_SAMPLE_HEADER + (SAMPLE_HEADER_LENGTH*c);
	
		sampleheader[c].length = pgm_read_byte(&moduledata[offset + 22]) << 9;
		sampleheader[c].length += pgm_read_byte(&moduledata[offset + 23]) << 1;

		soff += sampleheader[c].length;
	
		sampleheader[c].finetune = pgm_read_byte(&moduledata[offset + 24]);
		sampleheader[c].volume = (uint8_t)pgm_read_byte(&moduledata[offset + 25]);

		sampleheader[c].loop_start = (uint8_t)pgm_read_byte(&moduledata[offset + 26]) << 9;
		sampleheader[c].loop_start += (uint8_t)pgm_read_byte(&moduledata[offset + 27]) << 1;

		sampleheader[c].loop_length = (uint8_t)pgm_read_byte(&moduledata[offset + 28]) << 9;
		sampleheader[c].loop_length = (uint8_t)pgm_read_byte(&moduledata[offset + 29]) << 1;
		sampleheader[c].loop = (sampleheader[c].loop_length > 2);

		sampleheader[c].loop_end = sampleheader[c].loop_start + sampleheader[c].loop_length;

	}

}



/**
 *
 */
uint8_t get_highest_pattern_number() {
	
	uint8_t max = 0;
	uint32_t offset;
	offset = OFFSET_SONG_HEADER + 2;

	uint8_t i;
	for(i=0;i<128;i++) {
		uint8_t tmp = pgm_read_byte(&moduledata[offset + i]);	
		if (tmp > max) max = tmp;
	}

	return max;
	
}

/**
 *
 */
void copy_songinfo() {
	numpatterns = get_highest_pattern_number() + 1;
	numsongpos = pgm_read_byte(&moduledata[OFFSET_SONG_HEADER]);	


	uint8_t i;
	for(i=0;i<128;i++) {
		patterntable[i] = pgm_read_byte(&moduledata[OFFSET_SONG_HEADER + 2 + i]);
	}

}


/**
 *
 */
void modplayer_init() {	

	copy_sampleinfo();

	copy_songinfo();

	offset_samples = OFFSET_PATTERNS + (numpatterns * 1024);		

	// process the first row/frame
	process_row();
	process_frame();

}


