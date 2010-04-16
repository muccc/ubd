#ifndef __UB_RS485MESSAGE_H_
#define __UB_RS485MESSAGE_H_
#include "ubconfig.h"

void rs485msg_init(void);
uint8_t rs485msg_put(uint8_t data);
inline uint8_t * rs485msg_getMsg(void);
inline uint8_t rs485msg_getLen(void);
inline uint8_t rs485msg_getType(void);
#endif
