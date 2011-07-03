#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "digitalinput.h"
#include "ub.h"
#include "ubeeprom.h"
#include "ubaddress.h"

struct inputport{
    uint8_t *pinreg;
    uint8_t *portreg;
    uint8_t pin;
    //uint8_t *interruptreg;
    //uint8_t interruptbit;
    uint8_t pinname[20];
    uint8_t name[20];
    uint8_t pullup;
    uint8_t interrupt;
};

#define INPUTCOUNT 4 
struct inputportsconfig{
    uint8_t version;
    struct inputport ports[INPUTCOUNT];
};

struct inputportsconfig inputconfig_record EEMEM;
struct inputportsconfig inputconfig =
{5,
{{(uint8_t*)&PINB,/*(uint8_t*)&PCMSK0,0,*/(uint8_t*)&PORTB,PB1,"PB1","PB1",1,0},
{(uint8_t*)&PINC,/*(uint8_t*)&PCMSK0,0,*/(uint8_t*)&PORTC,PC0,"PC0","PC0",1,1},
{(uint8_t*)&PINC,/*(uint8_t*)&PCMSK0,0,*/(uint8_t*)&PORTC,PC1,"PC1","PC1",1,0},
{(uint8_t*)&PINC,/*(uint8_t*)&PCMSK0,0,*/(uint8_t*)&PORTC,PC6,"PC6","PC6",1,0}}
};

uint8_t lastvalue[INPUTCOUNT];
#define OUT_NONE    0
#define OUT_INTERRUPT   1
uint8_t out = OUT_NONE;

uint8_t outinterrupt_value;
uint8_t *outinterrupt_name;

void digitalinput_init()
{
    uint8_t i;
    ubeeprom_read(&inputconfig,&inputconfig_record,sizeof(inputconfig));
    for(i=0; i<INPUTCOUNT; i++){
        digitalinput_enablePullup(i);
        digitalinput_prepareInterrupt(i);
    }
    //Seems not very portable
    //PCICR |= (1<<PCIE0) | (1<<PCIE1) | (1<<PCIE2) | (1<<PCIE3);
}

void digitalinput_process(void)
{
    if( !out ){ 
        uint8_t i;
        for(i=0; i<INPUTCOUNT; i++){
            if( inputconfig.ports[i].interrupt ){
                uint8_t v = digitalinput_getValue(i);
                if( v != lastvalue[i] ){
                    //TODO: send packet
                    outinterrupt_value = v;
                    outinterrupt_name = inputconfig.ports[i].name;
                    lastvalue[i] = v;
                    out = OUT_INTERRUPT;
                    break;
                }
            }
        } 
    }
    if( out == OUT_INTERRUPT ){
        if( ubpacket_acquireUnsolicited(UB_CLASS_DIGITALINPUT) ){
            if( !ubpacket_isUnsolicitedDone() ){
                struct ubpacket_t *p = ubpacket_getSendBuffer();
                p->header.src = ubadr_getAddress();
                p->header.dest = UB_ADDRESS_MASTER;
                p->header.flags = UB_PACKET_UNSOLICITED;
                p->header.class = UB_CLASS_DIGITALINPUT;
                sprintf((char*)p->data,"I %s %u",outinterrupt_name, outinterrupt_value);
                p->header.len = strlen((char*)p->data);
                ubpacket_send();
            }else{
                out = OUT_NONE;
                ubpacket_releaseUnsolicited(UB_CLASS_DIGITALINPUT);
            }
        }
    }

}

void digitalinput_save()
{
    ubeeprom_write(&inputconfig,&inputconfig_record,sizeof(inputconfig));
}

void digitalinput_cmd(struct ubpacket_t * cmd, struct ubpacket_t * out)
{
    uint8_t n=255;
    struct inputport *p;
    uint8_t *data = cmd->data;
    uint8_t *parameter = NULL;
    uint8_t len = cmd->header.len;

    if( len == 0 )
        return;
    if( len == sizeof(cmd->data) )  //we need a little space at the end
        return;
    data[len] = 0;

    if(len > 2 && data[1] == ' '){
        uint8_t *name = data+2;
        parameter = (uint8_t*)strchr((char*)name,' ');
        if( parameter != NULL ){
            *parameter = 0;
            parameter++;
        }
        for(n=0; n<INPUTCOUNT; n++){
            if(strcmp((char*)name, (char*)inputconfig.ports[n].pinname) == 0)
                break;
            if(strcmp((char*)name, (char*)inputconfig.ports[n].name) == 0)
                break;
        }
        if( n == INPUTCOUNT ){
            if( len > 3 && data[3] && data[3] != ' ')
                n = (cmd->data[2]-'0')*10+cmd->data[3]-'0';
            else
                n = cmd->data[2]-'0';
        }
    }
    
    switch( data[0] ){
        case GET_NUMBEROFINPUTS:
            sprintf((char*)out->data, "%u",INPUTCOUNT);
            out->header.len = strlen((char*)out->data);
        break;
        case GET_INPUTINFO:
            if( n < INPUTCOUNT){
                p = &(inputconfig.ports[n]);
                sprintf((char*)out->data,"%s,%s,%u,%u,%u",
                        p->pinname, p->name, p->pullup, p->interrupt,
                        digitalinput_getValue(n));
                out->header.len = strlen((char*)out->data);
            }
        break;
        case SET_INPUTNAME:
            if( n < INPUTCOUNT && parameter != NULL){
                if( strlen((char*)parameter) < sizeof(inputconfig.ports[n].name) ){
                    strcpy((char*)inputconfig.ports[n].name, (char*)parameter);
                }
            }
            digitalinput_save();
        break;
        case SET_PULLUPENABLED:
            if( n < INPUTCOUNT && parameter != NULL){
                if( *parameter ){
                    if( *parameter == '0' ){
                        inputconfig.ports[n].pullup = 0;
                    }else if( *parameter == '1' ){
                        inputconfig.ports[n].pullup =1;
                    }
                    digitalinput_enablePullup(n);
                }
            }
            digitalinput_save();
        break;
        case SET_INTERRUPTENABLED:
            if( n < INPUTCOUNT && parameter != NULL){
                if( *parameter ){
                    if( *parameter == '0' ){
                        inputconfig.ports[n].interrupt = 0;
                    }else if( *parameter == '1' ){
                        inputconfig.ports[n].interrupt =1;
                    }
                    digitalinput_prepareInterrupt(n);
                }
            }
            digitalinput_save();
        break;
        case GET_INPUT:
            if( n < INPUTCOUNT ){
                p = &(inputconfig.ports[n]);
                sprintf((char*)out->data,"%u",
                        digitalinput_getValue(n));
                out->header.len = strlen((char*)out->data); 
            }
        break;
    }
}

void digitalinput_enablePullup(uint8_t n)
{
    if( n < INPUTCOUNT && inputconfig.ports[n].pullup ){
        uint8_t *portreg = inputconfig.ports[n].portreg;
        uint8_t pin = inputconfig.ports[n].pin;
        if( inputconfig.ports[n].pullup ){
            *portreg |= (1<<pin);
        }else{
            *portreg &= ~(1<<pin);
        }
    }
}

uint8_t digitalinput_getValue(uint8_t n)
{
    if( n < INPUTCOUNT ){
        uint8_t *pinreg = inputconfig.ports[n].pinreg;
        uint8_t pin = inputconfig.ports[n].pin;
        if( *pinreg & (1<<pin) )
            return 1;
        return 0;
    }else{
        return 0;
    }
}

void digitalinput_prepareInterrupt(uint8_t n)
{
    if( n < INPUTCOUNT ){
        lastvalue[n] = digitalinput_getValue(n);
    }
}


/*
void digitalinput_prepareInterrupt(uint8_t n)
{
     if( n < INPUTCOUNT ){
        uint8_t *interruptreg = inputconfig.ports[n].interruptreg;
        uint8_t interruptbit = inputconfig.ports[n].interruptbit;
        uint8_t enabled = inputconfig.ports[n].interrupt;
        if( enabled == 0){
            *interruptreg &= ~(1<<interruptbit);
        }else{
            *interruptreg |= (1<<interruptbit);
        }
    }
}

void digitalinput_checkInterrupt(uint8_t n)
{
    switch(n){
        case 0:
            
}

SIGNAL(PCINT0_vect)
{
    digitalinput_checkInterrupt(0);
}

SIGNAL(PCINT1_vect)
{
    digitalinput_checkInterrupt(1);
}

SIGNAL(PCINT2_vect)
{
    digitalinput_checkInterrupt(2);
}

SIGNAL(PCINT3_vect)
{
    digitalinput_checkInterrupt(3);
}
*/
