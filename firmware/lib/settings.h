#ifndef __SETTINGS_H_
#define __SETTINGS_H_
#include <stdint.h>


struct settings_record_t {
    uint8_t firstboot;
};
extern struct settings_record_t global_settings;

//extern uint8_t idbuf[60];

void settings_save(void);
void settings_read(void);
void settings_readid(uint8_t * buf);
uint8_t settings_compareid(uint8_t * buf);
void settings_setid(uint8_t * buf);
#endif
