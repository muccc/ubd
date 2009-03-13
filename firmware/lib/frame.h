#ifndef __FRAME_H_
#define __FRAME_H_
#include <stdint.h>

#define FRAME_MAX 50

struct frame {
    volatile uint8_t len;
    volatile uint8_t data[FRAME_MAX];
    volatile uint16_t crc;
    volatile uint8_t new;
};

#endif
