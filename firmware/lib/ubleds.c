#include <avr/io.h>
#include <avr/interrupt.h>

#include "ubleds.h"
#include "ubconfig.h"
#include "pinutils.h"

void ubleds_init(void)
{
#ifdef RXLED
    DDR_CONFIG_OUT(RXLED); 
#endif
#ifdef TXLED
    DDR_CONFIG_OUT(TXLED); 
#endif
}

inline void ubleds_rx(void)
{
#ifdef RXLED
    uint8_t sreg = SREG; cli();
    PIN_SET(RXLED);
    SREG = sreg;
#endif
}

inline void ubleds_rxend(void)
{
#ifdef RXLED
    uint8_t sreg = SREG; cli();
    PIN_CLEAR(RXLED);
    SREG = sreg;
#endif
}

inline void ubleds_tx(void)
{
#ifdef TXLED
    uint8_t sreg = SREG; cli();
    PIN_SET(TXLED);
    SREG = sreg;
#endif
}

inline void ubleds_txend(void)
{
#ifdef TXLED
    uint8_t sreg = SREG; cli();
    PIN_CLEAR(TXLED);
    SREG = sreg;
#endif
}
