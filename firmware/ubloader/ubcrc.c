#include <util/crc16.h>
#include "ubcrc.h"

/*uint16_t ubcrc16_frame(struct frame * f)
{
    uint16_t crc = 0xFFFF;
    uint8_t i = 0;

    crc = _crc_ccitt_update(crc, f->len);
    for(i=0;i<f->len;i++){
        crc = _crc_ccitt_update(crc, f->data[i]);
    }

    return crc;
}*/

uint16_t ubcrc16_data(uint8_t *data, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    uint8_t i = 0;

    //crc = _crc_ccitt_update(crc, len);
    //tx('A');
    for(i=0; i<len; i++){
        //tx(data[i]);
        crc = _crc_ccitt_update(crc, data[i]);
    }

    //tx('B');
    return crc;
}
