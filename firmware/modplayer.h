#include <avr/io.h>


struct t_sampleheader {
	uint32_t length;
	int8_t finetune;
	uint8_t volume;
	uint8_t loop;
	uint32_t loop_start;
	uint32_t loop_length;
	uint32_t loop_end;
	uint16_t data_offset;
};

struct t_channelstate {
	uint8_t sample_id;
	uint16_t rate_before_fx;
	uint8_t arp[3];
};


extern struct t_sampleheader sampleheader[31];
extern struct t_channelstate channelstate[4];

extern uint32_t offset_samples;

void modplayer_init();
void modplayer_next_frame();
