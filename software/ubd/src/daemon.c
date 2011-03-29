#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <syslog.h>
#include <libutil.h>

void daemon_init(void)
{
    /* Our process ID and Session ID */
    pid_t pid, sid;
    
    struct pidfh *pfh;
    pid_t otherpid;

    pfh = pidfile_open("/var/run/ubd.pid", 0600, &otherpid);
    if( pfh == NULL ){
        if( errno == EEXIST ){
            syslog(LOG_ERR,"Other deamon allready running.");
            exit(EXIT_FAILURE);
        }
        syslog(LOG_ERR,"Cannot open or create  pidfile.");
    }

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
            exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
        we can exit the parent process. */
    if (pid > 0) {
            exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);
    
    pidfile_write(pfh);

    /* Open any logs here */        
            
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
    }
    
    /* Change the current working directory */
    if ((chdir("/")) < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
    }
    
    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
 }
 
void daemon_close_stderror(void)
{
    close(STDERR_FILENO);
}
