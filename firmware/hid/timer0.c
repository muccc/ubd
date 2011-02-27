#include <avr/interrupt.h>
#include <avr/io.h>

volatile uint8_t timebase = 0;
ISR(TIMER0_OVF_vect , ISR_NOBLOCK)
{
    timebase++;
}

void timer0_init(void)
{
    //18e6/256/64 ~ 1000
#if defined(__AVR_ATmega324P__) ||  defined(__AVR_ATmega644P__)
    TCCR0B = (1<<CS00) | (1<<CS01);           //divide by 64
    TIMSK0 |= 1<<TOIE0;         //enable timer interrupt
#else
    TCCR0 = (1<<CS00) | (1<<CS01);           //divide by 64
    TIMSK |= 1<<TOIE0;          //enable timer interrupt
#endif
}
