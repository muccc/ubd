#include <avr/eeprom.h>
#include <string.h>
#include "ubeeprom.h"

void ubeeprom_write(void *src, void *dst, size_t n)
{
    eeprom_write_block(src,dst,n);
}

void ubeeprom_read(void *dst, void *src, size_t n)
{
    uint8_t version = eeprom_read_byte(src);
    uint8_t newversion = ((uint8_t *)dst)[0];
    if( version == 255 || version < newversion ){
        eeprom_write_block(dst,src,n);
    }
    eeprom_read_block(dst, src, n);
}

