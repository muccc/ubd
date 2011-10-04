#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

void daemon_init(void);
void daemon_close_stderror(void);
#endif

