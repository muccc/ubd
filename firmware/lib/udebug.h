#ifndef _UDEBUG_H_
#define _UDEBUG_H_
#include "serial_handler.h"
#include "ubconfig.h"

void udebug_init(void);
inline void udebug_edge(void);
inline void udebug_rx(void);
inline void udebug_txon(void);
inline void udebug_txoff(void);

#ifdef USEDEBUG
#define UDEBUG(x) serial_sendFrames(x)
#else
#define UDEBUG(X) {}
#endif

#endif
