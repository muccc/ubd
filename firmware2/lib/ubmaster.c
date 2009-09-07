#include "ubmaster.h"
#include "ubconfig.h"
#include "settings.h"
#include "ubrs485master.h"
#include <avr/io.h>
uint16_t            ubm_ticks = 0;

void ubmaster_init(void)
{
    rs485master_init();
}

//1ms
inline void ubmaster_tick(void)
{
    ubm_ticks++;
    rs485master_tick();
}

inline void ubmaster_process(void)
{
    rs485master_process();
}
