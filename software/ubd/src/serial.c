#include <config.h>
#include <stdio.h>
#include <glib.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>
#include "debug.h"
#include "serial.h"
#include <unistd.h>
#include <errno.h>
#include <syslog.h>

uint8_t serial_buffer[SERIAL_BUFFERLEN];
int fd;
GTimeVal start;


static uint16_t serial_in(uint8_t data);

inline int serial_putc(uint8_t c)
{
    return write(fd,&c,1);
}

inline void serial_putcenc(uint8_t c)
{
    if(c == SERIAL_ESCAPE)
        serial_putc(SERIAL_ESCAPE);
    serial_putc(c);
}

void serial_putsenc(char * s)
{
    while(*s){
        if(*s == SERIAL_ESCAPE)
            serial_putc(SERIAL_ESCAPE);
        serial_putc(*s++);
    }
}

void serial_putenc(uint8_t * d, uint16_t n)
{
    uint16_t i;
    for(i=0;i<n;i++){
        if(*d == SERIAL_ESCAPE)
            serial_putc(SERIAL_ESCAPE);
        serial_putc(*d++);
    }
}

inline void serial_putStart(void)
{
    serial_putc(SERIAL_ESCAPE);
    serial_putc(SERIAL_START);
}

inline void serial_putStop(void)
{
    serial_putc(SERIAL_ESCAPE);
    serial_putc(SERIAL_END);
}

void serial_sendFrames(char * s)
{
    serial_putStart();
    serial_putsenc(s);
    serial_putStop();
}

void serial_sendFramec(uint8_t c)
{
    serial_putStart();
    serial_putcenc(c);
    serial_putStop();
}

static uint16_t serial_in(uint8_t data)
{
    static int fill = -1;
    static uint8_t escaped = 0;
    int tmp;
    /*syslog(LOG_DEBUG,"serial: in:");
    debug_hexdump(&data,1);
    syslog(LOG_DEBUG,"\n");*/

    if(data == SERIAL_ESCAPE){
        if(!escaped){
            escaped = 1;
            return 0;
        }
        escaped = 0;
    }else if(escaped){
        escaped = 0;
        if(data == SERIAL_START){
            g_get_current_time(&start);
            fill = 0;
            return 0;
        }else if( data == SERIAL_END){
            ub_assert(fill != -1);
            tmp = fill;
            fill = -1;
            return tmp;
        }
    }
    if( fill > -1 ){
        //TODO: prevent overflow!
        serial_buffer[fill++] = data;
    }else if(fill >= SERIAL_BUFFERLEN){
        fill = -1;
    }
    return 0;
}

void serial_readMessage(struct message * msg)
{
    uint8_t len;
    while( 1 ){
        uint8_t c;
        int rc = read(fd,&c,1);
        if( rc > 0){
            len = serial_in(c);
            if( len ){
                if( serial_buffer[0] != 'P' ){
                    syslog(LOG_DEBUG,"%ld.%04ld serial: new message: ->",start.tv_sec,start.tv_usec);
                    debug_hexdump(serial_buffer, len);syslog(LOG_DEBUG,"<-\n");
                }
                msg->len = len;
                if( sizeof(msg->data) >= len ){
                    memcpy(msg->data,serial_buffer,msg->len);
                    return;
                }else{
                    //TODO: log this error
                    ub_assert( sizeof(msg->data) >= len);
                }
            }
        }else if( rc == 0){
            syslog(LOG_WARNING,"timeout on serial line\n");
        }else if( rc < 0){
            syslog(LOG_ERR,"error while reading from serial line: rc=%d errno=%d\n",rc,errno);
        }
    }
}

void serial_writeMessage(struct message * outmsg)
{
    //syslog(LOG_DEBUG,"serial: write message: ->");debug_hexdump(outmsg->data, outmsg->len);
    //syslog(LOG_DEBUG,"<-\n");
    serial_putStart();
    serial_putenc((uint8_t*) outmsg->data, outmsg->len);
    serial_putStop();
    //tcdrain(fd);
    //tcflush(fd,TCOFLUSH);
}

void serial_switch(void)
{
    syslog(LOG_INFO,"switching serial node to bridge mode\n");
    serial_sendFrames("B");
    struct message m;
    //return;
    while(1){
        serial_readMessage(&m);
        if( m.len == 1 && m.data[0] == 'B' )
            break;
    }
    syslog(LOG_INFO,"done\n");
    //usleep(1000*1000*2);
}

int serial_open (char * device)
{
    fd = open(device, O_RDWR|O_NOCTTY);// |O_SYNC);//|O_NONBLOCK);
    if( fd == -1 ){
        return -1;
    }
    struct termios tio;
    bzero(&tio, sizeof(tio));
    tio.c_cflag = CREAD | CLOCAL | B115200 | CS8;
    tio.c_iflag = IGNPAR | IGNBRK;
    tio.c_oflag = 0;
    tio.c_lflag = 0;
    tio.c_cc[VTIME] = 100;
    tio.c_cc[VMIN]  = 1;
    tcsetattr (fd, TCSANOW, &tio);
    tcflush (fd, TCIFLUSH);
    tcflush (fd, TCOFLUSH); 
    return 0;
}

