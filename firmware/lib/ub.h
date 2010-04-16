#ifndef __UB_H_
#define __UB_H_
#include <stdint.h>
#include "ubpacket.h"

enum UB_INTERFACE { UB_NOIF, UB_RS485, UB_RF };

enum UB_ERR_CODE { UB_OK, UB_ERROR };
typedef uint8_t UBSTATUS;

enum UB_SLAVE_CODE { UB_MASTER, UB_SLAVE };
struct ub_config {
    uint8_t rs485master;
    uint8_t rs485slave;
    uint8_t master;
    uint8_t slave;
    uint8_t configured;
};

extern struct ub_config ubconfig;
void ub_init(uint8_t ubmode);
void ub_process(void);
void ub_tick(void);
inline uint8_t ub_getAddress(void);
UBSTATUS ub_sendPacket(struct ubpacket_t * packet);
uint8_t ub_getPacket(struct ubpacket_t * packet);
#endif
