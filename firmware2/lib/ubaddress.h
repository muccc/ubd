#ifndef UB_ADDRESS_H_
#define UB_ADDRESS_H_
#include <stdint.h>
#include "ub.h"
#include "ubconfig.h"

void ubadr_init(void);

inline uint8_t * ubadr_getID(void);
inline uint8_t ubadr_getIDLen(void);
uint8_t ubadr_compareID(uint8_t * buf);

void ubadr_readID(void);
void ubadr_setID(uint8_t * buf);


void ubadr_setAddress(ubaddress_t adr);
ubaddress_t ubadr_getAddress(void);

UBSTATUS ubadr_addMulticast(ubaddress_t adr);
UBSTATUS ubadr_removeMulticast(ubaddress_t adr);

inline uint8_t ubadr_isLocal(ubaddress_t adr);
uint8_t ubadr_isLocalMulticast(ubaddress_t adr);
inline uint8_t ubadr_isUnicast(ubaddress_t adr);
inline uint8_t ubadr_isBroadcast(ubaddress_t adr);
inline uint8_t ubadr_isMulticast(ubaddress_t adr);
#endif
