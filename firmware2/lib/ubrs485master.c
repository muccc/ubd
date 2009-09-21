#include <string.h>

#include "ubconfig.h"
#include "ubrs485master.h"
#include "ubstat.h"
#include "ubrs485uart.h"
#include "settings.h"
#include "ubtimer.h"
#include "ub.h"
#include "ubrs485message.h"
#include "udebug.h"

#define UB_HOSTADR          1
#define UB_MASTERADR        2

#define UB_TICKSPERBYTE             10
#define UB_INITIALDISCOVERINTERVAL  500

#define UB_QUERYINTERVALMIN      64      //in ms
#define UB_QUERYINTERVALMAX      1000    //in ms

#define UB_QUERYINTERVALMAXCOUNT    (UB_QUERYINTERVALMAX/UB_QUERYINTERVALMIN)

#define UB_MAXQUERY                 30

#define UB_RECEIVE              0
#define UB_SEND                 1

#define RS485M_STATE_INIT               1
#define RS485M_STATE_INITIALDISCOVER    2
#define RS485M_STATE_STOP               5
#define RS485M_NORMAL                   3
#define RS485M_DISCOVER                 4

#define RS485M_SLOTCOUNT                3
#define RS485M_QUERYSLOT                0
#define RS485M_DISCOVERSLOT             1
#define RS485M_PACKETSLOT               2

#define RS485M_BUS_IDLE              0
#define RS485M_BUS_SEND_START        1
#define RS485M_BUS_SEND_DATA         2
#define RS485M_BUS_SEND_STOP         3
#define RS485M_BUS_SEND_STOP2        4
#define RS485M_BUS_SEND_DONE         5
#define RS485M_BUS_SEND_TIMER        6
#define RS485M_BUS_RECV              7
#define RS485M_BUS_SEND_TIMER_WAIT   8

struct rs485m_slot {
    uint8_t     adr;
    uint8_t     state;
    uint8_t     *data;
    uint8_t     len;
    uint8_t     full;
};

struct rs485m_queryinterval {     // contains the intervals to query special nodes
    uint8_t adr;
    uint8_t interval;
    uint8_t counter;
};

struct rs485m_slot rs485m_slots[RS485M_SLOTCOUNT];
struct rs485m_queryinterval rs485m_query[UB_QUERYMAX];

uint8_t    rs485m_state = RS485M_STATE_INIT;
uint16_t   rs485m_ticks = 0;


//used in the interrupts
volatile uint8_t rs485m_start;
volatile uint8_t rs485m_len;
volatile uint8_t *rs485m_data;
volatile uint8_t rs485m_stop;

volatile uint8_t rs485m_busState;
volatile uint8_t rs485m_busmode;

volatile uint8_t rs485m_incomming;
//buffer for the slave id during a query
uint8_t rs485m_querybuf[1];

UBSTATUS rs485master_setQueryInterval(uint8_t adr, uint16_t interval)
{ 
    uint8_t i;
    //Check for correct intervalls
    if( interval < UB_QUERYINTERVALMIN || interval > UB_QUERYINTERVALMAX )
        return UB_ERROR;
    
    interval = interval / UB_QUERYINTERVALMIN;
    //find an unused query slot
    for(i=0; i< UB_MAXQUERY; i++){
        if( rs485m_query[i].interval == UB_QUERYINTERVALMAXCOUNT ){
            rs485m_query[i].adr = adr;
            rs485m_query[i].interval = interval;
            rs485m_query[i].counter = interval;
            return UB_OK;
        }
    }
    //no slot found
    return UB_ERROR;
}

void rs485master_init(void)
{
    uint8_t i;
    ubtimer_init();
    rs485uart_init( UART_BAUD_SELECT(RS485_BITRATE,F_CPU));
    //ubtimer_start(UB_TICKSPERBYTE);      //start+8+stop
    for(i=0; i<UB_MAXQUERY; i++){
        rs485m_query[i].adr = i;
        //rs485m_query[i].interval = UB_QUERYINTERVALMAXCOUNT; 
        rs485m_query[i].interval = UB_QUERYINTERVALMAXCOUNT/1; 
        rs485m_query[i].counter  = UB_QUERYINTERVALMAXCOUNT/1;
    }

    for(i=0; i<RS485M_SLOTCOUNT; i++){
        rs485m_slots[i].full = 0;
    }

    rs485m_busState = RS485M_BUS_IDLE;
    rs485m_busmode = UB_RECEIVE;

    rs485m_incomming = UB_NONE;
    rs485uart_enableReceive();
    
    PORTA ^= 0x02;
}

//check for incomming messages
/*void rs485master_processIncomming(void)
{
    switch(rs458m_incomming){
        case UB_START:          //a new packet was received
            
        break;
    }
}*/

uint8_t rs485master_getPacket(uint8_t * buffer)
{
    uint8_t len;
    if( rs485m_incomming == UB_START ){
        len = rs485msg_getLen();
        memcpy(buffer, rs485msg_getMsg(), len);
        rs485m_incomming = UB_NONE;
        return len;
    }else if( rs485m_incomming != UB_NONE ){
        //ignore these packages
        rs485m_incomming = UB_NONE;
    }
    return 0;
}

//try to send a query to a node
UBSTATUS rs485master_query(uint8_t adr)
{
    if( rs485m_slots[RS485M_QUERYSLOT].full )
        return UB_ERROR;
    rs485m_slots[RS485M_QUERYSLOT].adr = adr;
    rs485m_slots[RS485M_QUERYSLOT].full = 1;
    return UB_OK;
}

UBSTATUS rs485master_discover(void)
{
    if( rs485m_slots[RS485M_DISCOVERSLOT].full )
        return UB_ERROR;
    rs485m_slots[RS485M_DISCOVERSLOT].full = 1;
    return UB_OK;
}

void rs485master_querynodes(void)
{
    static uint8_t query = 0;
    static uint8_t querymax = 0;

    static uint8_t queryperiod = UB_QUERYINTERVALMIN;
    static uint8_t queryperiodmax =  UB_QUERYINTERVALMAXCOUNT;


    if( queryperiod-- == 0 ){
        query = UB_MAXQUERY - 1;
        queryperiod = UB_QUERYINTERVALMIN;

        //time to check the rest?
        if( queryperiodmax-- == 0 ){
            queryperiodmax =  UB_QUERYINTERVALMAXCOUNT;
            querymax = 1;
        }
    }

    if( query < UB_MAXQUERY ){
        if( rs485m_query[query].counter-- == 0 ){   //check this node
            if(rs485master_query(rs485m_query[query].adr) == UB_OK){
                rs485m_query[query].counter = rs485m_query[query].interval;
            }else{
                rs485m_query[query].counter = 1;
                query++;        //check again later
            }
        }
        query--;                //stops the check by underrun
    }

    //testing:
    //check the other nodes, one per timer

    if( querymax ){
        uint8_t flags = ubstat_getFlags(querymax);
        if( flags & (UB_KNOWN | UB_RS485 | UB_QUERYMAX) ){
            if(rs485master_query(querymax) == UB_ERROR){
                querymax--;     //check again later
            }
        }
        querymax++;
        if( querymax == UB_NODEMAX )
            querymax = 0;
    }
}

//send a frame with data.
UBSTATUS rs485master_send(uint8_t * data, uint8_t len)
{
    //the frame has to contain data
    if( len == 0 )
        return UB_ERROR;

    //use the free packet slot
    if( rs485m_slots[RS485M_PACKETSLOT].full )
        return UB_ERROR;

    rs485m_slots[RS485M_PACKETSLOT].data = data;
    rs485m_slots[RS485M_PACKETSLOT].len = len;
    rs485m_slots[RS485M_PACKETSLOT].full = 1;
    return UB_OK;
}

//1ms
inline void rs485master_tick(void)
{
    rs485m_ticks++;
    rs485master_querynodes();
}

UBSTATUS rs485master_idle(void)
{
    if( rs485m_busState == RS485M_BUS_IDLE )
        return UB_OK;
    return UB_ERROR;
}

inline void rs485master_process(void)
{
    switch( rs485m_state ){
        case RS485M_STATE_INIT:
            rs485m_state = RS485M_STATE_INITIALDISCOVER;
        break;
        case RS485M_STATE_INITIALDISCOVER:
            if( rs485m_ticks == UB_INITIALDISCOVERINTERVAL ){
                rs485m_ticks = 0;
                rs485master_discover();
            }
        break;
    }
    if( rs485master_idle() == UB_OK ){
        rs485master_runslot();
    }
}

void rs485master_runslot(void)
{
    //implement priorities by aranging these items
    if( rs485m_slots[RS485M_DISCOVERSLOT].full ){
        //just send the discover escape sequence
        //and start the receive windwo
        rs485master_start(UB_DISCOVER,NULL,0,0);
        rs485m_slots[RS485M_DISCOVERSLOT].full = 0;
    }else if( rs485m_slots[RS485M_QUERYSLOT].full ){
        //send the query escape followed by the slave id

        //buffer the slave id, the slot might get overwritten
        rs485m_querybuf[0] = rs485m_slots[RS485M_QUERYSLOT].adr;
        rs485master_start(UB_QUERY,rs485m_querybuf,1,0);
        rs485m_slots[RS485M_QUERYSLOT].full = 0;
    }else if( rs485m_slots[RS485M_PACKETSLOT].full ){
        rs485master_start(UB_START,rs485m_slots[RS485M_PACKETSLOT].data,
                rs485m_slots[RS485M_PACKETSLOT].len,UB_STOP);
        //this slot will be freed when UB_STOP has been transmitted
    }
}

void rs485master_start(uint8_t start, uint8_t * data, uint8_t len, uint8_t stop)
{
    rs485uart_enableTransmit();
    rs485m_busState = RS485M_BUS_SEND_START;
    rs485m_start = start;
    rs485m_stop = stop;
    rs485m_data = data;
    rs485m_len = len;
    rs485uart_putc(UB_ESCAPE);
}

//rx interrupt
inline void rs485master_rx(void)
{
    udebug_rx();
    uint16_t i = rs485uart_getc();
    if( !(i & UART_NO_DATA) ){
        //restart timeout
        ubtimer_start(2 * UB_TICKSPERBYTE);
        //if( i == 0xAA )
            //PORTA ^= 0x02;
        uint8_t c = i&0xFF;
        /*if( c == 0xAA ){
             ubtimer_stop();
            PORTA &=~ 0x02;
            rs485m_busState = RS485M_BUS_IDLE;          
        }*/

        i = rs485msg_put(c);
        if( i != UB_NONE ){
            rs485m_incomming = i;
            ubtimer_stop();
            PORTA &=~ 0x02;
            rs485m_busState = RS485M_BUS_IDLE;
        }
    }
}

inline void rs485master_edge(void)
{
    if( rs485m_busState == RS485M_BUS_SEND_TIMER ){
        if( rs485uart_lineActive() && rs485uart_isReceiving() ){
            PORTA ^= (1<<PA2);
            rs485m_busState = RS485M_BUS_RECV;
            rs485uart_edgeDisable();
            //return to RS485M_BUS_IDLE if nothing gets received(noise)
            ubtimer_start(2 * UB_TICKSPERBYTE);
        }else{
            //this was not a valid start
        }
    }else{
        //we can't start receiving
        rs485uart_edgeDisable();
    }
}

inline void rs485master_timer(void)
{
    //is this timeout still waiting for a response?
    if( rs485m_busState ==  RS485M_BUS_SEND_TIMER || rs485m_busState == RS485M_BUS_RECV ){
        //proceed to the next slot
        rs485m_busState = RS485M_BUS_IDLE;
        PORTA &=~ 0x02;
    }
    rs485uart_edgeDisable();
    ubtimer_stop();
}

inline void rs485master_setTimeout(void)
{
    //wait for 4 bytes before timeout
    ubtimer_start(2 * UB_TICKSPERBYTE);      //start+8+stop
    rs485m_busState = RS485M_BUS_SEND_TIMER;
    PORTA |= 0x02;
    rs485uart_edgeEnable();
}

inline void rs485master_tx(void)
{
    static uint8_t pos;
    static uint8_t escaped;
    uint8_t data;

    switch(rs485m_busState){
        case RS485M_BUS_SEND_START:
            rs485uart_putc(rs485m_start);
            pos = 0;
            
            if( rs485m_len != 0)
                rs485m_busState = RS485M_BUS_SEND_DATA;
            else if( rs485m_stop != 0 )
                rs485m_busState = RS485M_BUS_SEND_STOP;
            else
                rs485m_busState = RS485M_BUS_SEND_TIMER_WAIT;
        break;
        case RS485M_BUS_SEND_DATA:
            data = rs485m_data[pos];
            if( data == UB_ESCAPE && !escaped){
                //insert an escape into the datastream
                rs485uart_putc(UB_ESCAPE);
                escaped = 1;
            }else{
                escaped = 0;
                rs485uart_putc(data);
                if( ++pos == rs485m_len ){
                    if( rs485m_stop == 0 ){
                        //no stop specified.
                        //give the slave time to answer
                        rs485m_busState = RS485M_BUS_SEND_TIMER_WAIT;
                    }else{
                        //only reached if a packet with data was transmitted
                        rs485m_slots[RS485M_PACKETSLOT].full = 0;
                        rs485m_busState = RS485M_BUS_SEND_STOP;
                    }
                }
            }
        break;
        case RS485M_BUS_SEND_STOP:
            rs485uart_putc(UB_ESCAPE);
            rs485m_busState = RS485M_BUS_SEND_STOP2;
        break;
        case RS485M_BUS_SEND_STOP2:
             rs485uart_putc(rs485m_stop);
             rs485m_busState = RS485M_BUS_SEND_DONE;
        break;
        case RS485M_BUS_IDLE:
        default:
        break;
    }
}

inline void rs485master_txend(void)
{
    if( rs485m_busState == RS485M_BUS_SEND_TIMER_WAIT )
        rs485master_setTimeout();
    if( rs485m_busState == RS485M_BUS_SEND_DONE )
        rs485m_busState = RS485M_BUS_IDLE;
    if( rs485m_busState == RS485M_BUS_IDLE
            || rs485m_busState == RS485M_BUS_SEND_TIMER){
        rs485uart_enableReceive();
    }
}


