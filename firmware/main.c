#include <avr/interrupt.h>
#include <avr/io.h>

#include "global.h"
#include "modplayer.h"
#include "moduledata.h"
#include "paula_emu.h"


int main (void) {


	//Set TIMER0 (PWM OC0 Pin)
	DDRB |= (1<<3);
	TCCR0 |= (1<<WGM01|1<<WGM00|1<<COM01|1<<CS00);
	OCR0 = 128;

	//Set TIMER1 (Next Sample Timer)	
	TIMSK |= (1 << OCIE1A);
	TCCR1B |= (1<<WGM12) | (1<<CS11);
		
	//Set TIMER2 (Frames)
	TIMSK |= (1 << OCIE2);
	TCCR2 = (1<<WGM21) | (1<<CS22) | (1<<CS21) | (1<<CS20);
	OCR2 = 255;

	modplayer_init();

	sei();
	
	while(1) { }

}


ISR (TIMER1_COMPA_vect) {	
	paula_render();	
}


ISR (TIMER2_COMP_vect) {	
	modplayer_next_frame();
}


