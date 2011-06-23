#ifndef __ADC_H_
#define __ADC_H_
#include <stdint.h>

void adc_init(void);
uint16_t adc_getChannel(uint8_t channel);
#endif
