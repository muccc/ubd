#include "ubconfig.h"
#include "ubrs485master.h"
#include "ubstat.h"
#include "ubuart.h"
#include "settings.h"

#define UB_HOSTADR          1
#define UB_MASTERADR        2

#define UB_TICKSPERBYTE             10
#define UB_INITIALDISCOVERINTERVAL  100


#define UB_QUERYINTERVALMIN      64      //in ms
#define UB_QUERYINTERVALMAX      1000    //in ms

#define UB_QUERYINTERVALMAXCOUNT    (UB_QUERYINTERVALMAX/UB_QUERYINTERVALMIN)

#define UB_MAXQUERY                 30

#define RS485M_STATE_INIT              1
#define RS485M_STATE_INITIALDISCOVER   2
#define RS485M_NORMAL                  3
#define RS485M_DISCOVER                4

struct queryinterval {     // contains the intervals to query special nodes
    uint8_t adr;
    uint8_t interval;
    uint8_t counter;
};

struct queryinterval rs485m_query[UB_QUERYMAX];
volatile uint8_t    rs485m_counter = 0;
uint8_t             rs485m_state = RS485M_STATE_INIT;
uint16_t            rs485m_ticks = 0;

uint8_t rs485master_setQueryInterval(uint8_t adr, uint16_t interval)
{ 
    uint8_t i;
    //Check for correct intervalls
    if( interval < UB_QUERYINTERVALMIN || interval > UB_QUERYINTERVALMAX )
        return UB_ERROR;
    
    interval = interval / UB_QUERYINTERVALMIN;
    //find an unused query slot
    for(i=0; i< UB_MAXQUERY; i++){
        if( rs485m_query[i].interval == UB_QUERYINTERVALMAXCOUNT ){
            rs485m_query[i].adr = adr
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
    ubtimer_start(UB_TICKPERBYTE);      //start+8+stop
    for(i=0; i< UB_MAXQUERY; i++){
        rs485m_query[i].adr = i;
        rs485m_query[i].interval = UB_QUERYINTERVALMAXCOUNT; 
        rs485m_query[i].counter  = UB_QUERYINTERVALMAXCOUNT;
    }
}

//try to send a query to a node
uint8_t rs485master_query(uint8_t adr)
{
    if( slots[RS485M_QUERYSLOT].full )
        return UB_ERROR;
    slots[RS485M_QUERYSLOT].adr = adr;
    slots[RS485M_QUERYSLOT].full = 1;
}

//1ms
void rs485master_tick(void)
{
    static uint8_t query = 0;
    static uint8_t querymax = 0;

    static uint8_t queryperiod = UB_QUERYINTERVALMIN;
    static uint8_t queryperiodmax =  UB_QUERYINTERVALMAXCOUNT;

    rs485m_ticks++;

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

    //check the other nodes, one per timer
    if( querymax ){
        uint8_t flags = ubstat_getFlags(querymax);
        if( flags & (UB_KNOWN | UB_RS485 | UB_QUERYMAX) ){
            if(rs485master_query(querymax) == UB_ERROR){
                querymax--;     //check again later
            }
        }
        querymax++;
    }

}

void rs485master_process(void)
{
    switch( rs485m_state ){
        case RS485M_STATE_INIT:
            rs485m_state = RS485M_STATE_INITIALDISCOVER;
        break;
        case RS485M_STATE_INITIALDISCOVER:
            if( rs485m_ticks == UB_INITIALDISCOVERINTERVAL){
                rs485m_ticks = 0;
                rs485master_start(UB_DISCOVER,NULL,0,0);
                //just send the discover escape sequence
            }
        break;
    }
}

void rs485master_start(uint8_t start, uint8_t * data, uint8_t len, uint8_t stop)
{
    UBUART_ENABLE();
    UBUART_PUTC(UB_ESCAPE);
    rs485m_start = start;
    rs485m_stop = stop;
    rs485m_data = data;
    rs485m_len = len;
    rs485m_sendState = RS485M_SEND_START;
}

inline void rs485master_rx(void)
{
    
}

inline void rs485master_edge(void)
{
    
}

inline void rs485master_timer(void)
{
    rs485m_counter++;
}

inline void rs485master_tx(void)
{
    static uint8_t pos;
    static uint8_t escaped;
    uint8_t data;

    switch(rs485m_sendState){
        case RS485M_SEND_START:
            UBUART_PUTC(UB_ESCAPE);
            pos = 0;
            
            if( len != 0)
                rs485m_sendState = RS485M_SEND_DATA;
            else if( rs485m_stop != 0 )
                rs485m_sendState = RS485M_SEND_STOP:
            else
                rs485m_sendState = RS485M_SEND_DONE;
        break;
        case RS485M_SEND_DATA:
            data = rs485m_data[pos];
            if( data == UB_ESCAPE && !escaped){
                UBUART_PUTC(UB_ESCAPE);
                escaped = 1;
            }else{
                escaped = 0;
                UBUART_PUTC(data);
                if( ++pos == rs485m_len ){
                    rs485m_sendState = RS485M_SEND_STOP;
                }
            }
        break;
        case RS485M_SEND_STOP:
            UBUART_PUTC(UB_ESCAPE);
            rs485m_sendState = RS485M_SEND_STOP2;
        break;
        case RS485M_SEND_STOP2:
             UBUART_PUTC(rs485m_stop);
             rs485m_sendState = RS485M_SEND_DONE;
        break;
        default:
        break;
    }
}

inline void rs485master_txend(void)
{
    if(rs485m_sendState == RS485M_SEND_DONE || rs485m_sendState == RS485M_SEND_IDLE){
        UBUART_DISABLE();
    }
}


