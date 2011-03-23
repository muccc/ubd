#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "config.h"
#include "fnordlicht.h"
#include "adc.h"
#include "leds.h"

void adc_init(void)
{
    ADMUX = (1<<REFS1) | (1<<REFS0);    //internal 2.56V
    //ADC on, single conversation, 150khz
    ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    adc_getChannel(6);
    adc_getChannel(6);
    adc_getChannel(6);
}


uint16_t adc_getChannel(uint8_t channel)
{
    //uint8_t sreg = SREG; cli();
    ADMUX &= 0xF0;
    ADMUX |= channel;
    ADCSRA |= (1<<ADSC);
    while( ADCSRA & (1<<ADSC) );
    //SREG = sreg;
    return ADC;

}
