#include <stdint.h>
#include <stdlib.h>

#include "random.h"
void random_init(uint8_t * seed, uint8_t len)
{
    uint8_t i,r;
    for(i=0; i<len; i++){
        r = random()&0xFF;
        srandom(r^seed[i]);
    }
}

inline uint8_t random_get(void)
{
    return random() & 0xFF;
}

