#include <stdint.h>
#include <string.h>
#include <avr/eeprom.h>

#include "ub.h"
#include "ubaddress.h"
#include "ubconfig.h"
#include "ubpacket.h"

char * eeprom_id = (char *)(E2END - 50);

ubaddress_t ubadr_adr;
ubaddress_t ubadr_multicast[UB_MAXMULTICAST];

uint8_t ubadr_id[30];
uint8_t ubadr_idlen;

inline uint8_t * ubadr_getID(void)
{
    return ubadr_id;
}

inline uint8_t ubadr_getIDLen(void)
{
    return ubadr_idlen;
}

void ubadr_readID(void)
{
    eeprom_read_block(ubadr_id,eeprom_id,30);
    ubadr_id[29] = 0;
    if(ubadr_id[0] == 0xFF){
         ubadr_setID((uint8_t*)UB_INITIALNODENAME);
    }
    ubadr_idlen = strlen((char*)ubadr_id);
}

void ubadr_setID(uint8_t * buf)
{
    uint8_t len = strlen((char*)buf);
    if(len > 30){
        len = 29;
        buf[len] = 0;
    }
    eeprom_write_block(buf,eeprom_id,len+1);
    ubadr_readID();
}

uint8_t ubadr_compareID(uint8_t * buf)
{
    if(strcmp((char*)ubadr_id,(char*)buf) == 0)
        return 1;
    return 0;
}

void ubadr_init(void)
{
    ubadr_readID();
}

inline void ubadr_setAddress(ubaddress_t adr)
{
    ubadr_adr = adr;
}

inline ubaddress_t ubadr_getAddress(void)
{
    return ubadr_adr;
}

UBSTATUS ubadr_addMulticast(ubaddress_t adr)
{
    uint8_t i;
    if( !ubadr_isMulticast(adr) )
        return UB_ERROR;
    for(i=0; i<UB_MAXMULTICAST; i++){
        if( ubadr_multicast[i] == 0 ){
            ubadr_multicast[i] = adr;
            return UB_OK;
        }
    }
    return UB_ERROR;
}

UBSTATUS ubadr_removeMulticast(ubaddress_t adr)
{
    uint8_t i;
    for(i=0; i<UB_MAXMULTICAST; i++){
        if( ubadr_multicast[i] == adr ){
            ubadr_multicast[i] = 0;
            return UB_OK;
        }
    }
    return UB_ERROR;
}

inline uint8_t ubadr_isLocal(ubaddress_t adr)
{
    return ubadr_adr == adr;
}

uint8_t ubadr_isLocalMulticast(ubaddress_t adr)
{
    uint8_t i;
    for(i=0; i<UB_MAXMULTICAST; i++){
        if( ubadr_multicast[i] == adr ){
            return 1;
        }
    }
    return 0;
}

inline uint8_t ubadr_isBroadcast(ubaddress_t adr)
{
    return adr == UB_ADDRESS_BROADCAST;
}

inline uint8_t ubadr_isMulticast(ubaddress_t adr)
{
    return (adr & UB_ADDRESS_MULTICAST) && (!ubadr_isBroadcast(adr));
}

inline uint8_t ubadr_isUnicast(ubaddress_t adr)
{
    return !ubadr_isBroadcast(adr) && !ubadr_isMulticast(adr);
}

