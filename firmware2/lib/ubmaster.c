#include "ububmaster.h"
#include "ubuart.h"
#include "settings.h"

#define UB_HOSTADR          1
#define UB_MASTERADR        2

#define UB_TICKSPERBYTE             10
#define UB_INITIALDISCOVERINTERVAL  100

#define UBM_STATE_INIT              1
#define UBM_STATE_INITIALDISCOVER   2
#define UBM_NORMAL                  3
#define UBM_DISCOVER                4



volatile uint8_t    ubm_counter = 0;
uint8_t             ubm_state = UBM_STATE_INIT;
uint16_t            ubm_ticks = 0;

void ubmaster_init(void)
{
    ubtimer_init();
    ubtimer_start(UB_TICKPERBYTE);      //start+8+stop
}

//1ms
void ubmaster_tick(void)
{
    ubm_ticks++;    
}

void ubmaster_process(void)
{
    switch( ubm_state ){
        case UBM_STATE_INIT:
            ubm_state = UBM_STATE_INITIALDISCOVER;
        break;
        case UBM_STATE_INITIALDISCOVER:
            if( ubm_ticks == UB_INITIALDISCOVERINTERVAL){
                ubm_ticks = 0;
                ubmaster_start(UB_DISCOVER,NULL,0,0);
            }
        break;
    }
}

void ubmaster_start(uint8_t start, uint8_t * data, uint8_t len, uint8_t stop)
{
    UBUART_ENABLE();
    UBUART_PUTC(UB_ESCAPE);
    ubm_start = start;
    ubm_stop = stop;
    ubm_data = data;
    ubm_len = len;
    ubm_sendState = UBM_SEND_START;
}

inline void ubmaster_rx(void)
{
    
}

inline void ubmaster_tx(void)
{
    static uint8_t pos;
    static uint8_t escaped;
    uint8_t data;

    switch(ubm_sendState){
        case UBM_SEND_START:
            UBUART_PUTC(UB_ESCAPE);
            pos = 0;
            
            if( len != 0)
                ubm_sendState = UBM_SEND_DATA;
            else if( ubm_stop != 0 )
                ubm_sendState = UBM_SEND_STOP:
            else
                ubm_sendState = UBM_SEND_DONE;
        break;
        case UBM_SEND_DATA:
            data = ubm_data[pos];
            if( data == UB_ESCAPE && !escaped){
                UBUART_PUTC(UB_ESCAPE);
                escaped = 1;
            }else{
                escaped = 0;
                UBUART_PUTC(data);
                if( ++pos == ubm_len ){
                    ubm_sendState = UBM_SEND_STOP;
                }
            }
        break;
        case UBM_SEND_STOP:
            UBUART_PUTC(UB_ESCAPE);
            ubm_sendState = UBM_SEND_STOP2;
        break;
        case UBM_SEND_STOP2:
             UBUART_PUTC(ubm_stop);
             ubm_sendState = UBM_SEND_DONE;
        break;
        default:
        break;
    }
}

inline void ubmaster_txend(void)
{
    if(ubm_sendState == UBM_SEND_DONE || ubm_sendState == UBM_SEND_IDLE){
        UBUART_DISABLE();
    }
}

inline void ubmaster_edge(void)
{
    
}

inline void ubmaster_timer(void)
{
    ubm_counter++;
}
