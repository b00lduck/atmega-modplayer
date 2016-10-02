#include <avr/io.h>
#include <avr/interrupt.h>

#include "modplayer_config.h"
#include "modplayer_paula_emu.h"

extern uint8_t playing_chan;

#include "../uart.h"

struct t_paulachannel paulachannel0;
struct t_paulachannel paulachannel1;
struct t_paulachannel paulachannel2;
struct t_paulachannel paulachannel3;

void paula_init() {

	//Set TIMER0 (PWM OC0A Pin)
	DDRB |= (1<<3);
	TCCR0A |= (1<<COM0A1) | (1<<COM0B1) | (1<<WGM01) | (1<<WGM00);
	TCCR0B |= (1<<CS00);
	OCR0A = 127;
	OCR0B = 127;

	//Set TIMER1 (Next Sample Timer)	
	TIMSK1 &= ~(1 << OCIE1A);	
	TCCR1B |= (1<<WGM12) | (1<<CS11 );
	TCNT1 = 0;
	OCR1A = 1500;

}


static inline int8_t get_next_sample(struct t_paulachannel* pch) {

	pch->position += pch->step;

	// LOOP
	if (pch->loop_enable) {			
		if (pch->position >= pch->loop_end) {
			pch->position -= (pch->loop_length);
		}
	} else
	 if (pch->position >= pch->length) {
		pch->playing = 0;
	}
	
	int8_t ret = (int8_t)READ_SONGDATA_BYTE(pch->addr + pch->position);

	return ret;

}

uint16_t lowest;

static inline void paula_procchan(struct t_paulachannel* pch) {

	if (pch->next_sample_in <= 0) {
		pch->next_sample_in += pch->finalrate;
		pch->out = get_next_sample(pch) * pch->volume;
	}

	if (pch->next_sample_in < lowest) {
		lowest = pch->next_sample_in;
	}	

}

uint16_t xxx = 0;

uint16_t maxdur = 0;

static inline void paula_render() {
	 
	//uint16_t start = TCNT1;

	lowest = 0x7fff;

	if (paulachannel0.playing) paula_procchan(&paulachannel0);
	if (paulachannel1.playing) paula_procchan(&paulachannel1);
	if (paulachannel2.playing) paula_procchan(&paulachannel2);
	if (paulachannel3.playing) paula_procchan(&paulachannel3);

	/*uint16_t stop = TCNT1;
	uint16_t dur = stop - start;

    if (maxdur < dur) {
		maxdur = dur;		
	}

	if (xxx >= 2000) { 
		uart_putw_dec(dur); 
		uart_putc(' ');
		uart_putw_dec(maxdur); 
		uart_putc('\n');
		xxx=0;
		maxdur = 0;
	}
	xxx++;

//	TCNT1 = stop;
*/

	if (lowest < LOWEST_STEP) lowest = LOWEST_STEP;

	OCR1A = lowest;

	paulachannel0.next_sample_in -= lowest;
	paulachannel1.next_sample_in -= lowest;
	paulachannel2.next_sample_in -= lowest;
	paulachannel3.next_sample_in -= lowest;

	int16_t tmp1 = paulachannel0.out + paulachannel3.out;
	int16_t tmp2 = paulachannel1.out + paulachannel2.out;

	uint8_t outl = (tmp1 >> 8) + 127;
	uint8_t outr = (tmp2 >> 8) + 127;

	OCR0A = outl;
	OCR0B = outr;

	return;
}



ISR (TIMER1_COMPA_vect) {	
	paula_render();	
}




