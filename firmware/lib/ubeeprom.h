#ifndef __EEPROM_H_
#define __EEPROM_H_
#include <avr/eeprom.h>
#include <stdint.h>
void ubeeprom_write(void *src, void *dst, size_t n);
void ubeeprom_read(void *dst, void *src, size_t n);
#endif
