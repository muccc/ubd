#include <stdint.h>
#include "ubrs485slave.h"
#include "ubtimer.h"
#include "ubrs485uart.h"
#include "ubrs485message.h"
#include "udebug.h"
#include "ub.h"

enum RS485S_BUSMODE{
    RS485S_BUS_IDLE,
    RS485S_BUS_RECV,
    RS485S_BUS_RECV_DONE,
    RS485S_BUS_SEND_START,
    RS485S_BUS_SEND_DATA,
    RS485S_BUS_SEND_STOP,
    RS485S_BUS_SEND_STOP2,
    RS485S_BUS_SEND_DONE
};

#define RS485S_SLOTCOUNT    1

#define RS485S_PACKETSLOT   0

struct rs485s_slot {
    uint8_t     adr;
    uint8_t     state;
    uint8_t     *data;
    uint8_t     len;
    uint8_t     full;
};

struct rs485s_slot rs485s_slots[RS485S_SLOTCOUNT];

volatile uint8_t rs485s_busState;
volatile uint8_t rs485s_start;
volatile uint8_t rs485s_stop;
volatile uint8_t * rs485s_data;
volatile uint8_t rs485s_len;

uint8_t testmsg[] = "slave2342tomaster";
uint8_t testlen = 17;

void rs485slave_init(void)
{
    ubtimer_init();
    rs485uart_init( UART_BAUD_SELECT(RS485_BITRATE,F_CPU));
    rs485uart_enableReceive();
    rs485s_busState = RS485S_BUS_IDLE;
}

inline void rs485slave_tick(void)
{
    
}

inline void rs485slave_process(void)
{
    if( rs485s_busState == RS485S_BUS_RECV_DONE ){
        uint8_t msgtype = rs485msg_getType();
        //if( i == UB_QUERY && rs485msg_getMsg()[0] == 0x10){
        if( msgtype == UB_START  && 
                rs485msg_getLen() == 6 && 
                rs485msg_getMsg()[5]%10 == 3 ){
            PORTA ^= 0x01;
        }
        rs485s_busState = RS485S_BUS_IDLE;
    }
}

inline void rs485s_gotQuery(void)
{
    PORTA ^= 0x02;
    if( rs485s_slots[RS485S_PACKETSLOT].full ){
        rs485slave_start(UB_START,
                rs485s_slots[RS485S_PACKETSLOT].data,
                rs485s_slots[RS485S_PACKETSLOT].len,
                UB_STOP);
    }
}

inline void rs485s_gotDiscover(void)
{
    PORTA ^= 0x04;
}

inline void rs485s_gotStart(void)
{
    rs485s_busState = RS485S_BUS_RECV_DONE;
}

inline void rs485slave_rx(void)
{
    udebug_rx();
    uint16_t i = rs485uart_getc();
    if( !(i & UART_NO_DATA) ){
        uint8_t c = i&0xFF;
         i = rs485msg_put(c);
         if( i == UB_START ){
            rs485s_gotStart();
         }else if( i == UB_QUERY && rs485msg_getMsg()[0] == ub_getAddress() ){
            rs485s_gotQuery();
         }else if( i == UB_DISCOVER ){ 
            rs485s_gotDiscover();
         }
    }
}

UBSTATUS rs485client_send(uint8_t * data, uint8_t len)
{
    if( len == 0 )
        return UB_ERROR;

    //use the free packet slot
    if( rs485s_slots[RS485S_PACKETSLOT].full )
        return UB_ERROR;

    rs485s_slots[RS485S_PACKETSLOT].data = data;
    rs485s_slots[RS485S_PACKETSLOT].len = len;
    rs485s_slots[RS485S_PACKETSLOT].full = 1;

    return UB_OK;
}

inline void rs485slave_edge(void)
{
    
}

inline void rs485slave_timer(void)
{
    
}

void rs485slave_start(uint8_t start, uint8_t * data, uint8_t len, uint8_t stop)
{
    rs485uart_enableTransmit();
    rs485s_busState = RS485S_BUS_SEND_START;
    rs485s_start = start;
    rs485s_stop = stop;
    rs485s_data = data;
    rs485s_len = len;
    rs485uart_putc(UB_ESCAPE);
}

inline void rs485slave_tx(void)
{
    static uint8_t pos;
    static uint8_t escaped;
    uint8_t data;

    switch(rs485s_busState){
        case RS485S_BUS_SEND_START:
            rs485uart_putc(rs485s_start);
            pos = 0;
            rs485s_busState = RS485S_BUS_SEND_DATA;
        break;
        case RS485S_BUS_SEND_DATA:
            data = rs485s_data[pos];
            if( data == UB_ESCAPE && !escaped){
                //insert an escape into the datastream
                rs485uart_putc(UB_ESCAPE);
                escaped = 1;
            }else{
                escaped = 0;
                rs485uart_putc(data);
                if( ++pos == rs485s_len ){
                    rs485s_slots[RS485S_PACKETSLOT].full = 0;
                    rs485s_busState = RS485S_BUS_SEND_STOP;
                }
            }
        break;
        case RS485S_BUS_SEND_STOP:
            rs485uart_putc(UB_ESCAPE);
            rs485s_busState = RS485S_BUS_SEND_STOP2;
        break;
        case RS485S_BUS_SEND_STOP2:
             rs485uart_putc(rs485s_stop);
             rs485s_busState = RS485S_BUS_SEND_DONE;
        break;
        case RS485S_BUS_IDLE:
        default:
        break;
    }
}

inline void rs485slave_txend(void)
{
    if( rs485s_busState == RS485S_BUS_SEND_DONE ){
        rs485s_busState = RS485S_BUS_IDLE;
        rs485uart_enableReceive();
    }
}


