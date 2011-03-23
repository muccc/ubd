#include <string.h>
#include <avr/interrupt.h>

#include "ubconfig.h"
#include "ubrf.h"
#include "ubstat.h"
#include "settings.h"
#include "ubtimer.h"
#include "ub.h"
#include "udebug.h"
#include "ubpacket.h"
#include "ubcrc.h"
#include "ubaddress.h"
#include "ubrf12.h"
#include "random.h"
#include "udebug.h"
#include "ubleds.h"

enum UBRF_STATE {   UBRF_IDLE,
                    UBRF_WAITFREE,
                    UBRF_WAITRND,
                    UBRF_SEND };
uint8_t ubrf_state;
uint8_t packetdata[UB_PACKETLEN+2];    //+ 2 byte crc
uint8_t packetlen;

void ubrf_init(void)
{
    ubleds_rx();
    ubrf_state = UBRF_IDLE;
    ubrf12_init(RF_CHANNEL);
    ubrf12_setfreq(RF12FREQ(434.32));
    ubrf12_setbandwidth(4, 1, 4);     // 200kHz Bandbreite,
    //-6dB Verstärkung, DRSSI threshold: -79dBm
    ubrf12_setbaud(19200);
    ubrf12_setpower(0, 6);            // 1mW Ausgangangsleistung,120kHz Frequenzshift
    ubrf12_rxstart();
    ubleds_rxend();
}

UBSTATUS ubrf_getPacket(struct ubpacket_t * packet)
{
    uint8_t *buf = (uint8_t*)packet;
    uint8_t len = ubrf12_rxfinish(buf);
    //PORTC ^= (1<<PC1);
    if( len == 255){
        return UB_ERROR;
    }
    if( len < 3 ){
        ubrf12_rxstart();
        return UB_ERROR;
    }
    //PORTC ^= (1<<PC2);
    uint16_t crc = ubcrc16_data(buf, len-2);
    if( (crc>>8) == buf[len-2] &&
        (crc&0xFF) == buf[len-1] ){
        ubrf12_rxstart();
        return UB_OK;
    }
    UDEBUG("DRF CRC WRONG");
    ubrf12_rxstart();
    return UB_ERROR;
}

//send a frame with data.
UBSTATUS ubrf_sendPacket(struct ubpacket_t * packet)
{
    if( ubrf_state == UBRF_IDLE ){
        ubrf_state = UBRF_WAITFREE;

        uint8_t len = packet->header.len
                + sizeof(packet->header);
        memcpy(packetdata, packet, len);

        uint16_t crc = ubcrc16_data(
                packetdata, len);
        packetdata[len++] = crc>>8;
        packetdata[len++] = crc&0xFF;
        packetlen = len;
        return UB_OK;
    }
    UDEBUG("Drferror");
    return UB_ERROR;
}

//1ms
inline void ubrf_tick(void)
{
    static uint8_t wait;
    ubrf12_rxstart();
    switch(ubrf_state){
        case UBRF_IDLE:
        break;
        case UBRF_WAITFREE:
            if( ubrf12_free() ){
                ubrf_state = UBRF_WAITRND;
                //TODO: use a backoff
                wait = random_get()&0xF;
            }
        break;
        case UBRF_WAITRND:
            if( wait-- == 0 ){
                if( ubrf12_free() ){
                    //TODO: check len(+header)?
                    ubrf12_allstop();
                    ubrf12_txstart(
                        packetdata, packetlen);
                    ubrf_state = UBRF_SEND;
                }else{
                    ubrf_state = UBRF_WAITFREE;
                }
            }
        break;
        case UBRF_SEND:
            if( ubrf12_txfinished() ){
                ubrf_state = UBRF_IDLE;
                ubrf12_rxstart();
            }
        break;
    }
}

//can we accept a packet?
UBSTATUS ubrf_free(void)
{
    if( ubrf_state == UBRF_IDLE )
        return UB_OK;
    return UB_ERROR;
}

inline void ubrf_process(void)
{
}

