#ifndef __UB_H_
#define __UB_H_
#include <stdint.h>
#include "ubpacket.h"

enum UB_INTERFACE { UB_NOIF  = 0, 
                    UB_RS485 = 1,
                    UB_RF    = 2
                  };

enum UB_ERR_CODE { UB_OK, UB_ERROR };
typedef uint8_t UBSTATUS;

enum UB_SLAVE_CODE { UB_BRIDGE, UB_SLAVE };
struct ub_config {
    uint8_t rs485master;
    uint8_t rs485slave;
    uint8_t bridge;
    uint8_t slave;
    uint8_t configured;
    uint8_t rf;
};

#define UB_ADDRESS_MASTER       1
#define UB_ADDRESS_BRIDGE       2

#define UB_ADDRESS_BROADCAST     ((1<<sizeof(ubaddress_t)*8)-1)     //all ones
#define UB_ADDRESS_MULTICAST     (1<<(sizeof(ubaddress_t)*8-1))     //first bit is one


extern struct ub_config ubconfig;
void ub_init(uint8_t ubmode, int8_t slaveinterfaces, int8_t bridgeinterfaces);
void ub_process(void);
void ub_tick(void);
inline uint8_t ub_getAddress(void);
UBSTATUS ub_sendPacket(struct ubpacket_t * packet);
uint8_t ub_getPacket(struct ubpacket_t * packet);
uint16_t ub_getTimeout(void);
#endif
