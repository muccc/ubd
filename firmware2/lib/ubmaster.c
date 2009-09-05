#include "ubmaster.h"
#include "ubconfig.h"
#include "settings.h"
#include "usart.h"
uint16_t            ubm_ticks = 0;

void ubmaster_init(void)
{
}

//1ms
void ubmaster_tick(void)
{
    ubm_ticks++;    
}

