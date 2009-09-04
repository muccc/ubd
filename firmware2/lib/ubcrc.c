#include "ubcrc.h"
#include "ubconfig.h"

uint16_t ubcrc16_frame(struct frame * f)
{
    uint16_t crc = 0xFFFF;
    uint8_t i = 0;

    crc = _crc_ccitt_update(crc, f->len);
    for(i=0;i<f->len;i++){
        crc = _crc_ccitt_update(crc, f->data[i]);
    }

    return crc;
}
