#ifndef _DIGITALINPUT_H_
#define _DIGITALINPUT_H_
#include "ubpacket.h"

#define GET_NUMBEROFINPUTS      'G'
#define GET_INPUTINFO           'g'
#define GET_INPUT               'i'

#define SET_INPUTNAME           'n'
#define SET_PULLUPENABLED       'p'
#define SET_INTERRUPTENABLED    'I'


void digitalinput_init(void);
void digitalinput_process(void);
void digitalinput_cmd(struct ubpacket_t * cmd, struct ubpacket_t * out);
void digitalinput_enablePullup(uint8_t n);
uint8_t digitalinput_getValue(uint8_t n);
void digitalinput_prepareInterrupt(uint8_t n);
void digitalinput_save(void);
 
#endif
