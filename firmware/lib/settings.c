#include <avr/eeprom.h>
#include <string.h>

#include "settings.h"

//struct global_t global_record EEMEM;
struct settings_record_t global_settings_record EEMEM = {1};

struct settings_record_t global_settings;

char * name = (char *)(E2END - 50);
uint8_t idbuf[60];

void settings_readid(uint8_t * buf)
{
    //eeprom_read_block(buf,name,50);
    strcpy(buf,idbuf);//"newid.local");
}

uint8_t settings_compareid(uint8_t * buf)
{
    if(strcmp((char*)idbuf,(char*)buf) == 0)
        return 1;
    return 0;
}

void settings_setid(uint8_t * buf)
{
    buf[30] = 0;
    uint8_t len = strlen((char*)buf);
//    if(len > (sizeof(id)-1)){
//        len = sizeof(id)-1;
/*    if(len > 50){
        len = 49;
        buf[len] = 0;
    }*/
    eeprom_write_block(buf,name,len+1);  
}

void settings_save(void)
{
    //const void * temp;
    global_settings.firstboot = 0;
    eeprom_write_block(&global_settings, &global_settings_record,sizeof(global_settings)); 
//    temp = (const void *) &global;
//    eeprom_write_block(/*(struct global_t *)&global*/temp,&global_record,sizeof(global));
}

void settings_read(void)
{
    //void * temp;
    eeprom_read_block(&global_settings, &global_settings_record,sizeof(global_settings));
    if(global_settings.firstboot){
    }else{
//        temp = (void *) &global;
//        eeprom_read_block(/*(struct global_t *)&global*/temp,&global_record,sizeof(global));
    }
    eeprom_read_block(idbuf,name,50);
    if(idbuf[0] == 255){
        strcpy((char*)idbuf,"newid.local");
        eeprom_write_block(idbuf,name,50);
    }
}

