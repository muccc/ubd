#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <avr/io.h>

#include "rf12config.h"
#include "rf12.h"
#include "rf12packet.h"
#include "packet.h"
#include "interfaces.h"

//#include "uart.h"

uint8_t state = STATE_IDLE;
uint8_t rf12packet_status = 0;
uint8_t count = 0;
uint8_t retries = 0;
uint8_t outlen = 0;
uint8_t acked = 0;
uint8_t seq = 0;
uint8_t isbroadcast = 0;
uint8_t rf12packet_sniff = 0;
uint8_t doreply = 0;

struct packet_t rf12packet;
struct rf12ack_t    expectedack;
struct rf12packet_t outpacket;
struct rf12packet_t inpacket;

void rf12packet_init(unsigned char adr)
{
//    localadr = adr;
}

/*void rf12packet_setadr(unsigned char adr)
{
//    localadr = adr;
}*/

/*unsigned char rf12packet_getstatus(void)
{
    return rf12packet_status;
}*/

void rf12packet_sendpacket(void)
{
//  cli();
    rf12_allstop();
    unsigned char c = rf12_txstart((uint8_t *)&outpacket,outlen);
//  sei();
    if(c){
#ifdef RF12DEBUG
        printf("sendpacket ret: %u\r\n", c);
#endif
#ifdef RF12DEBUGBIN
        printf("acDsendpaacket ret: %uab", c);
#endif
    }
}

void rf12packet_tick(void)      //every 1ms ~ 2bytes
{
//    EIMSK &= ~(1<<INT0);
    unsigned char ret = rf12_rxfinish((uint8_t *)&inpacket);
    if(ret != 255 && ret != 254 && ret != 0){
        rf12packet_process(&inpacket,ret);
    }else{                                  //len== 0 || no new data ||
                                            //rx not finished || crc error
#ifdef RF12DEBUGBIN
        if(ret == 0)           //len or crc
            printf("acEcrc errorab");       //propably crc
#endif
    }
/*    if(rf12_txfinished() == 0){
        rf12_rxstart();
        
    }else
        return;
        */
    if(rf12_txfinished()){          //!=0 when transmitting
//        EIMSK |= (1<<INT0);
        return;
    }
    switch(state){
        case STATE_IDLE:
            rf12_rxstart();         //aborts if already receiving
        break;
        case STATE_SENDPACKET:
            if(retries--){
                state = STATE_WAITFREE;
            }else{
                rf12packet_status |= RF12PACKET_TIMEOUT;
                state = STATE_IDLE;
            }
        break;
        case STATE_WAITFREE:        //wait for clean air
            ret = ((rf12_trans(0x0000)>>8) & (1<<0));   //clock recovery bit
            if(!ret){
            //if(RF12_Index == 0){
                count = 3; //+ binexprnd[0...10];   //Todo: random exponential
                                                    //backoff needed
                state = STATE_WAITRND;              //wait some time(csma)
#ifdef RF12DEBUG
                printf("free\r\n");
#endif
#ifdef RF12DEBUGBIN
                printf("acDfreeab");
#endif
            }else{
#ifdef RF12DEBUG
                printf("busy\r\n");
#endif
#ifdef RF12DEBUGBIN
                printf("acDbusyab");
#endif
            }
        break;
        case STATE_WAITRND:
            if(--count == 0){                   //when waiting check again
                ret = ((rf12_trans(0x0000)>>8) & (1<<0));
                if(ret){
                //if(RF12_Index){
                    state = STATE_WAITFREE;     //busy air ...
#ifdef  RF12DEBUGBIN
                    printf("acDwaitab");
#endif
                }else{
#ifdef RF12DEBUGBIN
                    printf("acDsendab");
#endif
                    rf12packet_sendpacket();    //go kids, go
                    state = STATE_SEND;
                }
            }
        break;
        case STATE_SEND:
            if(rf12_txfinished() == 0){         //finished?
                rf12_rxstart();                 //recv ack
                count = 20;                     //max. 20ms pour le ack
                acked = 0;
                /*if(retries != 0)                //bb mc set this to zero
                    state = STATE_WAITACK;
                else
                    state = STATE_IDLE;*/
                if(isbroadcast){
                    rf12packet_status |= RF12PACKET_PACKETDONE;
#ifdef RF12DEBUGBIN
                    uart1_puts("acDBDoneab");
#endif
                    state = STATE_IDLE;
                }else{
                    state = STATE_WAITACK;
                }
            }
        break;
        case STATE_WAITACK:
            rf12_rxstart();             //we might have sent an ack
                                        //for a packet and turned the tx on
            if(acked){
                rf12packet_status |= RF12PACKET_PACKETDONE;     //tell the app
                rf12_rxstart();         //start rx again
                state = STATE_IDLE;
            }else{
                if(--count == 0){       //no ack after 20ms
                    state = STATE_SENDPACKET;
#ifdef RF12DEBUG
                    printf("nack\r\n");
#endif
#ifdef RF12DEBUGBIN
                    printf("acDnaackab");
#endif
                }
            }
        break;
    }

//    EIMSK |= (1<<INT0);
}
#define MAX_HOSTS   10
static unsigned char seqs[MAX_HOSTS][2];

unsigned char checkseq(unsigned char host, unsigned char seq)
{
    unsigned char i;
    unsigned char rc;
    static unsigned char seqrr = 0;
    for(i=0;i<MAX_HOSTS;i++){
        if(seqs[i][0] == host){                     //host known
            rc = seqs[i][1]!=seq?1:0;               //check for old seq
            seqs[i][1] = seq;
            return rc;
        }
    }                                               //host unknown
    seqs[seqrr][0] = host;                          //add host to table
    seqs[seqrr++][1] = seq;
    if(seqrr == MAX_HOSTS)                          //round robin style
        seqrr = 0;
    return 1;
}

void rf12packet_process(struct rf12packet_t  * inpacket, unsigned char len)
{
    //unsigned char buf[10];
    struct rf12ack_t ack;
    struct packet_t * packet = &(inpacket->packet);
    //uint8_t i;
/*    uart1_puts("acDNP");
    serial_putenc(inpacket,len);
    uart1_puts("ab");*/
    //uart1_puts("acDNPab");
    if(inpacket->type == 'A' && inpacket->seq == (unsigned char)(seq+1) ){
        /*if(packet_isOwnAddress(packet)){   //check for our ack
            acked = 1;
            seq++;
        }*/
        if(expectedack.src  == ((struct rf12ack_t *)inpacket)->src &&
           expectedack.dest == ((struct rf12ack_t *)inpacket)->dest){
            acked = 1;
            seq++;
           }
        //printf("acked\r\n");
    }
    if(inpacket->type == 'B'){
        if(packet_isOwnBroadcast(packet) || rf12packet_sniff){    //this is a broadcast for us
            /*if((len-RF12PACKET_HEADERLEN) > sizeof(rf12packet))
                len = sizeof(rf12packet);
            else
                len -= RF12PACKET_HEADERLEN;
            memcpy(&rf12packet,packet+RF12PACKET_HEADERLEN,len);*/

//            uart1_puts("acDNB");
            memcpy(&rf12packet,packet,packet->len+PACKET_HEADERLEN);
//            serial_putenc(&rf12packet,packet->len+PACKET_HEADERLEN);
//            uart1_puts("ab");
            rf12packet_status |= RF12PACKET_NEWDATA;
        }
    }

    if(inpacket->type == 'P'){
        if(packet_isOwnAddress(packet) || rf12packet_sniff){
//            uart_puts("acDAab");
            ack.type='A';
            ack.seq=inpacket->seq+1;
            //buf[2]=inpacket[3];
            ack.dest = packet->lasthop;
            //buf[3]=localadr;
            //buf[3]=inpacket[2];
            ack.src = packet->nexthop;
#ifdef RF12DEBUGBIN
//            printf("acDaacking\r\nab");
#endif
            rf12_allstop();                 //propably in rx ;)
            rf12_txstart((uint8_t *)&ack,sizeof(ack));
        }
        if(checkseq(packet->lasthop,inpacket->seq) || rf12packet_sniff){
//            uart_puts("acDPPab");
            memcpy(&rf12packet,packet,packet->len+PACKET_HEADERLEN);
            rf12packet_status |= RF12PACKET_NEWDATA;
        }
    }
}

unsigned char rf12packet_incrseq(void)      //used for not mc and bc packets
{                                           //as they don't get acked
    seq++;
    return seq;
}

uint8_t rf12packet_packetOut(struct packet_t * p)
{
    if(state != STATE_IDLE)
        return 1;
//    uart1_puts("acDPoab");
    state = STATE_SENDPACKET;
    if(p->flags & PACKET_BROADCAST){
        outpacket.type = 'B';
        isbroadcast = 1;
        retries = 1;
    }else{
        outpacket.type = 'P';
        isbroadcast = 0;
        retries = 10;
        expectedack.src = p->nexthop;
        expectedack.dest = p->lasthop;
    }
    if(p->flags & PACKET_REPLYDONE){
        doreply = 1;
        p->flags ^= PACKET_REPLYDONE;
    }

    outpacket.seq = seq;
    memcpy(&(outpacket.packet),p,p->len+PACKET_HEADERLEN);
    outlen = p->len+PACKET_HEADERLEN+RF12PACKET_HEADERLEN;
    rf12packet_status &= ~RF12PACKET_PACKETDONE;
    return 0;
}

uint8_t rf12packet_nextHeader(struct packet_t * p)
{
    struct packet_t * packet = &(outpacket.packet);
    if(doreply && 
        (rf12packet_status & RF12PACKET_PACKETDONE ||
         rf12packet_status & RF12PACKET_TIMEOUT)){
        p->len = 0;
        p->dest = packet->src;
        p->src = packet_getAddress();
        p->lasthop =  packet_getAddress();
        p->nexthop = packet_getAddress();
        p->iface = IFACE_NONE;
        if(rf12packet_status & RF12PACKET_TIMEOUT){
            p->flags = PACKET_TIMEOUT;
        }else{
            p->flags = PACKET_DONE;
        }
        //uart1_puts("acDWDab");
        return 1;
    }
    if(rf12packet_status & RF12PACKET_NEWDATA){
        memcpy(p,&rf12packet,PACKET_HEADERLEN);
        return 1;
    }
    return 0;
}

uint8_t rf12packet_packetIn(struct packet_t * p)
{
    //struct packet_t * packet = &(outpacket.packet);
    if(doreply &&
        (rf12packet_status & RF12PACKET_PACKETDONE ||
         rf12packet_status & RF12PACKET_TIMEOUT)){
            rf12packet_nextHeader(p);
            rf12packet_status &= ~( RF12PACKET_PACKETDONE | RF12PACKET_TIMEOUT);
            doreply = 0;
/*            uart1_puts("acDdoingreply");
            serial_putenc(p,PACKET_HEADERLEN);
            uart1_puts("ab");*/
            return 0;
    }
    if(!(rf12packet_status & RF12PACKET_NEWDATA))
        return 1;
    memcpy(p,&rf12packet,rf12packet.len + PACKET_HEADERLEN);
    rf12packet_status ^= RF12PACKET_NEWDATA;
    return 0;
}

uint8_t rf12packet_ready(void)
{
    return state==STATE_IDLE?1:0;
}

