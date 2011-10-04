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

#define BUF_SIZE 100
#include <fcntl.h>

/* Lock a file region (private; public interfaces below) */

static int
lockReg(int fd, int cmd, int type, int whence, int start, off_t len)
{
    struct flock fl;

    fl.l_type = type;
    fl.l_whence = whence;
    fl.l_start = start;
    fl.l_len = len;

    return fcntl(fd, cmd, &fl);
}

static int                     /* Lock a file region using nonblocking F_SETLK */
lockRegion(int fd, int type, int whence, int start, int len)
{
    return lockReg(fd, F_SETLK, type, whence, start, len);
}

void daemon_init(void)
{
    /* Our process ID and Session ID */
    pid_t pid, sid; 
    int fd;
    char buf[BUF_SIZE];
    char *pidFile = "/var/run/ubd.pid";
    char *progName = "ubd";

    /* already a daemon */
    // PID = 1: init
    if ( getppid() == 1 ) return;
    
    fd = open(pidFile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1){
            syslog(LOG_ERR,"Could not open PID file /var/run/ubd.pid");
            exit(EXIT_FAILURE);

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
 
    if (lockRegion(fd, F_WRLCK, SEEK_SET, 0, 0) == -1) {
        if (errno  == EAGAIN || errno == EACCES){
            syslog(LOG_ERR,"PID file '/var/run/ubd.pid' is locked; probably "
                     "'ubd' is already running");
            exit(EXIT_FAILURE);
        }else{
            syslog(LOG_ERR,"Unable to lock PID file '/var/run/ubd.pid'");
            exit(EXIT_FAILURE);
        }
    }

    if (ftruncate(fd, 0) == -1){
        syslog(LOG_ERR,"Could not truncate PID file '/var/run/ubd.pid'");
        exit(EXIT_FAILURE);
    }

    snprintf(buf, BUF_SIZE, "%ld\n", (long) getpid());
    if (write(fd, buf, strlen(buf)) != strlen(buf)){
        syslog(LOG_ERR,"Writing to PID file '/var/run/ubd.pid'");
        exit(EXIT_FAILURE);
    }
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
