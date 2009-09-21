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

volatile uint8_t rs485s_busmode;

void rs485slave_init(void)
{
    ubtimer_init();
    rs485uart_init( UART_BAUD_SELECT(RS485_BITRATE,F_CPU));
    rs485uart_enableReceive();
    rs485s_busmode = RS485S_BUS_IDLE;
}

inline void rs485slave_tick(void)
{
    
}

inline void rs485slave_process(void)
{
    if( rs485s_busmode == RS485S_BUS_RECV_DONE ){
        uint8_t msgtype = rs485msg_getType();
        //if( i == UB_QUERY && rs485msg_getMsg()[0] == 0x10){
        if( msgtype == UB_START  && 
                rs485msg_getLen() == 6 && 
                rs485msg_getMsg()[5]%10 == 3 ){
            PORTA ^= 0x01;
        }
        rs485s_busmode = RS485S_BUS_IDLE;
    }
}

inline void rs485s_gotQuery(void)
{
    PORTA ^= 0x02;

}

inline void rs485s_gotDiscover(void)
{
    PORTA ^= 0x04;
}

inline void rs485s_gotStart(void)
{
    rs485s_busmode = RS485S_BUS_RECV_DONE;
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

inline void rs485slave_tx(void)
{
    
}

inline void rs485slave_txend(void)
{
    
}

inline void rs485slave_edge(void)
{
    
}

inline void rs485slave_timer(void)
{
    
}
