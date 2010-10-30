#include <avr/interrupt.h>
#include <avr/io.h>

#include "global.h"

#include "modplayer.h"
#include "paula_emu.h"

extern 


int main (void) {

	init_modplayer();
	init_paula();

	//Set TIMER0 (PWM OC0 Pin)
	DDRB |= (1<<3);
	TCCR0 |= (1<<WGM01|1<<WGM00|1<<COM01|1<<CS00);
	OCR0 = 128;
	
	//Set TIMER2 (Samples)
	TIMSK |= (1 << TOIE2);
	TCCR2 = (1<<CS11);
	TCNT2 = 255 - (SYSCLK / 8 / SAMPLERATE);			

	sei();
	
	while(1) {	
	}

}


uint16_t samples_until_next_frame = SAMPLES_PER_FRAME;

/**
 *
 */
ISR (TIMER2_OVF_vect) {	
	TCNT2 = 255 - (SYSCLK / 8 / SAMPLERATE);	
	OCR0 = paula_render() + 128;

	if (samples_until_next_frame > 0) {
		samples_until_next_frame--;
	} else {
		samples_until_next_frame = SAMPLES_PER_FRAME;	
		next_frame();
	}
}


