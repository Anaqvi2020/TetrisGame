

#ifndef TIMER_H
#define TIMER_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>


volatile unsigned char TimerFlag = 0; 


unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerISR(void);

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void TimerOn() {
	
	TCCR2A = 0x00;
  TCCR2B 	= 0x0B;	
	OCR2A 	= 250;

	TIMSK2 	= 0x02; 

	//Initialize avr counter
	TCNT2 = 0;

	
	_avr_timer_cntcurr = _avr_timer_M;

	//Enable global interrupts
	SREG |= 0x80;
}

void TimerOff() {
	TCCR2B 	= 0x00; //timer is off here
}




ISR(TIMER2_COMPA_vect)
{
	
	_avr_timer_cntcurr--; 			
	if (_avr_timer_cntcurr == 0) { 	
		TimerISR(); 				
		_avr_timer_cntcurr = _avr_timer_M;
	}

}

int TimerOverflow = 0;

ISR(TIMER1_OVF_vect)
{
	TimerOverflow++;	
}


double read_sonar(){
    long count;
    
    PORTC |= 0x04;
    _delay_us(10);
    
    PORTC &= ~0x04;

    TCNT1 = 0;	
		TCCR1B = 0x41;	
		TIFR1 = 1<<ICF1;	
		TIFR1 = 1<<TOV1;	//Clear flag

		
		while ((TIFR1 & (1 << ICF1)) == 0);
		TCNT1 = 0;	/* Clear counter */
		TCCR1B = 0x01;	
		TIFR1 = 1<<ICF1;	
		TIFR1 = 1<<TOV1;	
		TimerOverflow = 0;  
		while ((TIFR1 & (1 << ICF1)) == 0);
		count = ICR1 + (65535 * TimerOverflow);	
		
		return((double)count / 932.46);
}


#endif 
