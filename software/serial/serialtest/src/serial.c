#include <config.h>
#include <stdio.h>
#include <gnet.h>
#include <glib.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>
#include "debug.h"
#include "serial.h"

uint8_t serial_buffer[255];
void (*serial_callback)(struct message *);
GIOChannel  * serial_io;

inline serial_putc(uint8_t c)
{
//    printf("serial: out:");
    g_io_channel_write_chars(serial_io, &c, 1, NULL, NULL);
//    debug_hexdump(&c,1);
//    printf("\n");
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

void serial_sendFramec(uint8_t s)
{
    serial_putStart();
    serial_putcenc(s);
    serial_putStop();
}

uint16_t serial_in(uint8_t data)
{
    static int fill = 0;
    static uint8_t escaped = 0;

//    printf("serial: in:");
//    debug_hexdump(&data,1);
//    printf("\n");

    if(data == SERIAL_ESCAPE){
        if(!escaped){
            escaped = 1;
            return 0;
        }
        escaped = 0;
    }else if(escaped){
        escaped = 0;
        if(data == SERIAL_START){
            fill = 0;
            return 0;
        }else if( data == SERIAL_END){
            return fill;
        }
    }
    serial_buffer[fill++] = data;
    if(fill >= SERIAL_BUFFERLEN)
        fill = SERIAL_BUFFERLEN - 1;
    return 0;
}

gboolean serial_read(GIOChannel * serial, GIOCondition condition, gpointer data)
{
    gsize n;
    uint8_t c;
    struct queues * q = data;
    uint16_t len;
    
    //try to read one byte
    int r = g_io_channel_read_chars(serial,&c,1,&n,NULL);

    if( n > 0 ){
        //feed it into the receiver
        len = serial_in(c);
        if( len ){
            //send the msg to the callback
//            printf("serial: new message\n");
            struct message * msg = g_new(struct message,1);
            msg->len = len;
            memcpy(msg->data,serial_buffer,msg->len);
            serial_callback(msg);
        }
    }else{
        //there was an error
        printf("result: %u\n",r);
    }
    
    //keep this source
    return TRUE;
}


void serial_writemessage(struct message * outmsg)
{
    printf("serial: write message: ");debug_hexdump(outmsg->data, outmsg->len);
    printf("\n");
    serial_putStart();
    serial_putenc((uint8_t*) outmsg->data, outmsg->len);
    serial_putStop();
    g_io_channel_flush(serial_io, NULL);
}

int serial_open (char * device, void (*cb)(struct message *))
{
    int fd = open(device, O_RDWR|O_NOCTTY);//|O_NONBLOCK);
    if( fd == -1 ){
//        printf("Failed to open serial device %s\n", argv[1]);
        return fd;
    }

    struct termios tio;
    tcgetattr (fd, &tio);
    tio.c_cflag = CREAD | CLOCAL | B115200 | CS8;
    tio.c_iflag = IGNPAR | IGNBRK;
    tio.c_oflag = 0;
    tio.c_lflag = 0;
    tio.c_cc[VTIME] = 0;
    tio.c_cc[VMIN]  = 0;
    tcsetattr (fd, TCSANOW, &tio);
    tcflush (fd, TCIFLUSH);
    tcflush (fd, TCOFLUSH);

    serial_callback = cb;

    serial_io = g_io_channel_unix_new(fd);
    g_io_channel_set_encoding(serial_io,NULL,NULL);

    g_io_add_watch(serial_io,G_IO_IN,serial_read,NULL);
    
    return fd;
}

