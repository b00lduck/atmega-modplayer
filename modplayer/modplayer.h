#include <avr/io.h>
#include "modplayer_paula_emu.h"

struct t_songinfo {
	char name[21];
	uint32_t offset_samples;
	uint32_t offset_songheader;
	uint32_t offset_patterns;
	uint8_t numpatterns;
	uint8_t numsongpos;
	uint8_t patterntable[128];
	uint8_t numsamples;
};

struct t_sampleheader {
	uint32_t length;
	int8_t finetune;
	uint8_t volume;
	uint8_t loop;
	uint32_t loop_start;
	uint32_t loop_length;
	uint32_t loop_end;
	uint32_t data_offset;
};

struct t_channelstate {
	uint8_t sample_id;
	uint16_t rate_before_fx;
	uint16_t FParp[3];
	int8_t volume_slide;
	uint8_t volume;
	int16_t pitch_slide;
};

extern struct t_sampleheader sampleheader[31];
extern struct t_channelstate channelstate[4];
extern struct t_songinfo songinfo;

void get_sample_name(char* buf, uint8_t id);
void modplayer_init();
//void modplayer_next_frame();
void trigger(uint8_t ch, struct t_paulachannel* pch, uint8_t sample_id, uint16_t rate);
void modplayer_play();
void modplayer_stop();





