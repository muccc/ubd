#include <avr/io.h>
#include <avr/interrupt.h>

#include "leds.h"
#include "config.h"
#include "pinutils.h"

void leds_init(void)
{
    DDR_CONFIG_OUT(LED1); 
    DDR_CONFIG_OUT(LED2); 
    DDR_CONFIG_OUT(LED3); 
    //PIN_SET(LED2);
}

inline void leds_rx(void)
{
    uint8_t sreg = SREG; cli();
    PIN_SET(LED2);
    SREG = sreg;
}


inline void leds_rxend(void)
{
    uint8_t sreg = SREG; cli();
    PIN_CLEAR(LED2);
    SREG = sreg;
}

inline void leds_adc(void)
{
//    uint8_t sreg = SREG; cli();
//    PIN_SET(LED1);
//    SREG = sreg;
}


inline void leds_adcend(void)
{
//    uint8_t sreg = SREG; cli();
//    PIN_CLEAR(LED1);
//    SREG = sreg;
}

inline void leds_tx(void)
{
    uint8_t sreg = SREG; cli();
    PIN_SET(LED3);
    SREG = sreg;
}


inline void leds_txend(void)
{
    uint8_t sreg = SREG; cli();
    PIN_CLEAR(LED3);
    SREG = sreg;
}

inline void leds_main(void)
{
    uint8_t sreg = SREG; cli();
    PIN_TOGGLE(LED1);
    SREG = sreg;
}
