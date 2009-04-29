#include <config.h>
#include <stdio.h>
#include <gnet.h>
#include <glib.h>
#include <fcntl.h>
#include <termios.h>

int main (int argc, char *argv[])
{
    printf ("This is " PACKAGE_STRING ".\n");
    int fd = open("/dev/ttyUSB0", O_RDWR|O_NOCTTY);//|O_NONBLOCK);
    struct termios tio;
    tcgetattr (fd, &tio);
    tio.c_cflag = CREAD | CLOCAL | B115200 | CS8;
    tio.c_iflag = IGNPAR | IGNBRK;
    tio.c_oflag = 0;
    tio.c_lflag = 0;
    tio.c_cc[VTIME] = 0;
    tio.c_cc[VMIN]  = 1;
    tcsetattr (fd, TCSANOW, &tio);
    tcflush (fd, TCIFLUSH);
    tcflush (fd, TCOFLUSH); 
    while(1){
        char buf[10];
        int n = read(fd,buf,1);
        printf("got len=%d char=%x\n",n,buf[0]);
    }
    //io = g_io_channel_unix_new(fd);
    return 0;
}

