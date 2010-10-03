
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



struct structChannelstate {
	uint8_t sample_id;
	uint32_t rate_before_fx;
	uint8_t arp[3];
} channelstate[4];

struct structSampleHeader {

	uint16_t length;
	int8_t finetune;
	uint8_t volume;
	uint8_t loop;
	uint16_t loop_start;
	uint16_t loop_length;
	uint16_t loop_end;
	uint16_t data_offset;

} sampleHeader[31];


void trigger(uint8_t ch, uint8_t sample_id, uint16_t rate) {

	channelstate[ch].sample_id = sample_id;
	channelstate[ch].rate_before_fx = rate;
	channelstate[ch].arp[0] = 0;

	paula_channel[ch].addr = offset_samples + sampleHeader[sample_id].data_offset;
	paula_channel[ch].FP16_length = ((uint32_t)sampleHeader[sample_id].length) << FPACBITS;
	paula_channel[ch].rate = rate;
	paula_channel[ch].volume = 64;

	paula_channel[ch].FP16_loop_start = ((uint32_t)sampleHeader[sample_id].loop_start) << FPACBITS;
	paula_channel[ch].FP16_loop_end = ((uint32_t)sampleHeader[sample_id].loop_end) << FPACBITS;
	paula_channel[ch].loop_enable = sampleHeader[sample_id].loop;

	paula_channel[ch].FP16_position = 0;
	paula_channel[ch].playing = 1;
	
}


/**
 *
 */
void next_row() {

	activerow++;

	if (activerow == 64) {
		songpos++;
		if(songpos >= numsongpos) songpos = 0;
		activerow = 0;
	}

}

/**
 *
 */
void process_row() {
	uint8_t ch = 0;

	uint32_t pre_offset = OFFSET_PATTERNS + (patterntable[songpos] << 10) + (activerow << 4);

	for (ch=0;ch<4;ch++) {

		// Decode one division
		uint32_t offset = pre_offset + (ch << 2);

		uint8_t byte3 = pgm_read_byte(&moduledata[offset+2]);
		uint8_t sample_id = (byte3 >> 4);
		uint16_t rate = (pgm_read_byte(&moduledata[offset]) << 8) + (pgm_read_byte(&moduledata[offset+1]));
		uint8_t effectdata = pgm_read_byte(&moduledata[offset+3]);
		uint8_t effectid = byte3 & 0x0f;	

		if (sample_id > 0) {
			trigger(ch,sample_id - 1,rate);
		}

		switch(effectid) {

			case 0:
				channelstate[ch].arp[0] = 0;
				channelstate[ch].arp[1] = effectdata >> 4;
				channelstate[ch].arp[2] = effectdata & 0xf;				
				break;

			case 15:
				speed = effectdata & 0xf;
				break;
		
		}
		
		
	}	

}

// 7bit FPA
//uint16_t FParptable[16] = {128,136,144,152,161,171,181,192,203,215,228,241,256,271,287,304};

// 8bit FPA
uint16_t FParptable[16] = {256,271,287,304,322,342,362,383,406,430,456,483,512,542,574,608};

//10bit FPA
//uint16_t FParptable[16] = {1024,1085,1149,1218,1290,1366,1448,1534,1625,1721,1823,1932,2047,2168,2297,2433};


//16bit FPA
//uint32_t FParptable[16] = {65536,69429,73553,77922,82551,87454,92649,98152,103982,110159,116702,123634,130978,138758,147001,155733};
 
/**
 *
 */
void process_frame() {

	//uint8_t arpindex = activeframe % 3;

	uint8_t ch;
	for (ch=0;ch<4;ch++) {

		//uint8_t arptable_index = channelstate[ch].arp[arpindex];

		//uint32_t tmp = (channelstate[ch].rate_before_fx) * FParptable[arptable_index];	
	
		//if (channelstate[ch].playing) {			
		//	channelstate[ch].FPrate = (FPratefactor / tmp) << 8;
		//}
	
	}


}

/**
 *
 */
void next_frame() {
	
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

		sampleHeader[c].data_offset = soff;
		uint32_t offset;
		offset = OFFSET_SAMPLE_HEADER + (SAMPLE_HEADER_LENGTH*c);
	
		sampleHeader[c].length = pgm_read_byte(&moduledata[offset + 22]) << 9;
		sampleHeader[c].length += pgm_read_byte(&moduledata[offset + 23]) << 1;

		soff += sampleHeader[c].length;
	
		sampleHeader[c].finetune = pgm_read_byte(&moduledata[offset + 24]);
		sampleHeader[c].volume = (uint8_t)pgm_read_byte(&moduledata[offset + 25]);

		sampleHeader[c].loop_start = (uint8_t)pgm_read_byte(&moduledata[offset + 26]) << 9;
		sampleHeader[c].loop_start += (uint8_t)pgm_read_byte(&moduledata[offset + 27]) << 1;

		sampleHeader[c].loop_length = (uint8_t)pgm_read_byte(&moduledata[offset + 28]) << 9;
		sampleHeader[c].loop_length = (uint8_t)pgm_read_byte(&moduledata[offset + 29]) << 1;
		sampleHeader[c].loop = (sampleHeader[c].loop_length > 2);


		sampleHeader[c].loop_end = sampleHeader[c].loop_start + sampleHeader[c].loop_length;

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
void init_modplayer() {	

	copy_sampleinfo();

	copy_songinfo();

	offset_samples = OFFSET_PATTERNS + (numpatterns * 1024);		

	// process the first row/frame
	process_row();
	process_frame();

}


