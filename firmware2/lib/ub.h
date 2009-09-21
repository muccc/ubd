#ifndef __UB_H_
#define __UB_H_
#include <stdint.h>

//#define UB_ERROR        1
//#define UB_OK           0

enum UB_ERR_CODE { UB_OK, UB_ERROR };
typedef uint8_t UBSTATUS;

//#define UB_MASTER       0
//#define UB_SLAVE       1

enum UB_SLAVE_CODE { UB_MASTER, UB_SLAVE };
struct ub_config {
    uint8_t rs485master;
    uint8_t rs485slave;
    uint8_t master;
    uint8_t slave;
};

extern struct ub_config ubconfig;
void ub_init(uint8_t ubmode);
void ub_process(void);
void ub_tick(void);
inline uint8_t ub_getAddress(void);
void ub_setAddress(uint8_t address);

#endif
