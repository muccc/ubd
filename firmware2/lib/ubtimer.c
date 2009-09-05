#include <avr/io.h>

#include "ubconfig.h"
#include "ubtimer.h"

#define UBTIMER_PRESCALER       32
#define UBTIMER_TICKSPERBIT     (F_CPU/RS485_BITRATE/UBTIMER_PRESCALER)



void ubtimer_init(void)
{
    TCCR2A = (1<<WGM21);        //CTC
    TCCR2B = 0;
    TIMSK2 = (1<<OCIE2A);
}

//generate an interrupt in t bittimes
void ubtimer_start(uint8_t t)
{
    TCNT2 = 0;
    OCR2A = UBTIMER_TICKSPERBIT * t;
    TCCR2B = (1<<CS21) | (1<<CS20);         // /32
}

void ubtimer_stop(void)
{ 
    TCCR2B = 0;         //stop
}

