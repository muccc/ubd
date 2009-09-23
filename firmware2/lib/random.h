#ifndef __RANDOM_H_
#define __RANDOM_H_
#include <stdint.h>

void random_init(uint8_t * seed, uint8_t len);
inline uint8_t random_get(void);

#endif
