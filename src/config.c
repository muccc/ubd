#include <syslog.h>
#include "config.h"

struct ubconfig config;

void config_init(void)
{
    config.nodetimeout = 120;
    config.rate = 115200;
    config.sysloglevel = LOG_DEBUG;
}
