#ifndef __LEDS_H_
#define __LEDS_H_
void leds_init(void);
inline void leds_rx(void);
inline void leds_rxend(void);
inline void leds_tx(void);
inline void leds_txend(void);
inline void leds_adc(void);
inline void leds_adcnd(void);
inline void leds_main(void);
#endif
