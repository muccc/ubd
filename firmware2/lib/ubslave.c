#include <stdint.h>
#include "ubslave.h"
#include "ubrs485slave.h"

uint8_t buf[30];
uint8_t ubslave_configured = 0;

void ubslave_init(void)
{
   rs485slave_init();
}

void ubslave_process(void)
{
    rs485slave_process();
    rs485s_getMessage(buf);
}

void ubslave_tick(void)
{
    rs485slave_tick();
}

