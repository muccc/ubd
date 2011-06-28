#ifndef _DIGITALOUTPUT_H_
#define _DIGITALOUTPUT_H_
#include "ubpacket.h"

#define GET_NUMBEROFOUTPUTS     'G'
#define GET_OUTPUTINFO          'g'

#define SET_OUTPUTNAME          'n'
#define SET_OUTPUTENABLED       'e'
#define SET_OUTPUTRESETVALUE    'r'
#define SET_OUTPUT              's'


void digitaloutput_init(void);
void digitaloutput_cmd(struct ubpacket_t * cmd, struct ubpacket_t * out);
void digitaloutput_enablePin(uint8_t n);
uint8_t digitaloutput_getValue(uint8_t n);
void digitaloutput_setValue(uint8_t n, uint8_t v);
void digitaloutput_save(void);
 
#endif
