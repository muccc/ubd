#include "ubslave.h"
#include "ubrs485slave.h"

void ubslave_init(void)
{
   rs485slave_init();
}

void ubslave_process(void)
{
    rs485slave_process();
}

void ubslave_tick(void)
{
    rs485slave_tick();
}

