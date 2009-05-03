#include "debug.h"
#include <stdio.h>

void debug_hexdump(uint8_t * data, uint16_t len)
{
    uint16_t i;
    for(i=0; i<len; i++){
        if( data[i] < 0x10 ){
            printf(" 0%X", data[i]);
        }else if (data[i] <= ' ' || data[i] > 0x7F){
            printf(" %X", data[i]);
        }else{
            printf("%c", data[i]);
        }
    }
}
