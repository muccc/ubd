#include <string.h>
#include <stdio.h>
#include "digitaloutput.h"
#include "ubeeprom.h"

struct port{
    uint8_t *ddr;
    uint8_t *port;
    uint8_t pin;
    uint8_t pinname[20];
    uint8_t name[20];
    uint8_t enabled;
    uint8_t resetvalue;
};

#define OUTPUTCOUNT 2 
struct portsconfig{
    uint8_t version;
    struct port ports[OUTPUTCOUNT];
};

struct portsconfig config_record EEMEM;
struct portsconfig config =
{1,
{{(uint8_t*)&DDRB,(uint8_t*)&PORTB,PB1,"PB1","PB1",1,1},
{(uint8_t*)&DDRD,(uint8_t*)&PORTD,PD4,"PD4","PD4",1,1}}
};

void digitaloutput_init()
{
    uint8_t i;
    ubeeprom_read(&config,&config_record,sizeof(config));
    for(i=0; i<OUTPUTCOUNT; i++){
        digitaloutput_enablePin(i);
    }
}

void digitaloutput_save()
{
    ubeeprom_write(&config,&config_record,sizeof(config));
}

void digitaloutput_enablePin(uint8_t n)
{
    if( n < OUTPUTCOUNT && config.ports[n].enabled ){
        uint8_t *ddr = config.ports[n].ddr;
        uint8_t pin = config.ports[n].pin;
        *ddr |= (1<<pin);
        digitaloutput_setValue(n, config.ports[n].resetvalue);
    }
}

uint8_t digitaloutput_getValue(uint8_t n)
{
    if( n < OUTPUTCOUNT ){
        uint8_t *port = config.ports[n].port;
        uint8_t pin = config.ports[n].pin;
        if( *port & (1<<pin) )
            return 1;
        return 0;
    }else{
        return 0;
    }
}

void digitaloutput_setValue(uint8_t n, uint8_t v)
{
    if( n < OUTPUTCOUNT ){
        uint8_t *port = config.ports[n].port;
        uint8_t pin = config.ports[n].pin;
        if( v ){
            *port |= (1<<pin);
        }else{
            *port &= ~(1<<pin);
        }
    }
}

void digitaloutput_cmd(struct ubpacket_t * cmd, struct ubpacket_t * out)
{
    uint8_t n=255;
    struct port *p;
    uint8_t *data = cmd->data;
    uint8_t *parameter = NULL;
    uint8_t len = cmd->header.len;

    if( len == 0 )
        return;
    if( len == sizeof(cmd->data) )  //we need little space at the end
        return;
    data[len] = 0;

    if(len > 2 && data[1] == ' '){
        uint8_t *name = data+2;
        parameter = (uint8_t*)strchr((char*)name,' ');
        if( parameter != NULL ){
            *parameter = 0;
            parameter++;
        }
        for(n=0; n<OUTPUTCOUNT; n++){
            if(strcmp((char*)name, (char*)config.ports[n].pinname) == 0)
                break;
            if(strcmp((char*)name, (char*)config.ports[n].name) == 0)
                break;
        }
        if( n == OUTPUTCOUNT ){
            if( len > 3 && data[3] && data[3] != ' ')
                n = (cmd->data[2]-'0')*10+cmd->data[3]-'0';
            else
                n = cmd->data[2]-'0';
        }
    }
    
    switch( data[0] ){
        case GET_NUMBEROFOUTPUTS:
            sprintf((char*)out->data, "%u",OUTPUTCOUNT);
            out->header.len = strlen((char*)out->data);
        break;
        case GET_OUTPUTINFO:
            if( n < OUTPUTCOUNT){
                p = &(config.ports[n]);
                sprintf((char*)out->data,"%s,%s,%u,%u,%u",
                        p->pinname, p->name, p->enabled, p->resetvalue,
                        digitaloutput_getValue(n));
                out->header.len = strlen((char*)out->data);
            }
        break;
        case SET_OUTPUTNAME:
            if( n < OUTPUTCOUNT && parameter != NULL){
                if( strlen((char*)parameter) < sizeof(config.ports[n].name) ){
                    strcpy((char*)config.ports[n].name, (char*)parameter);
                }
            }
            digitaloutput_save();
        break;
        case SET_OUTPUTENABLED:
            if( n < OUTPUTCOUNT && parameter != NULL){
                if( *parameter ){
                    if( *parameter == '0' ){
                        config.ports[n].enabled = 0;
                    }else if( *parameter == '1' ){
                        config.ports[n].enabled =1;
                    }
                    digitaloutput_enablePin(n);
                }
            }
            digitaloutput_save();
        break;
        case SET_OUTPUTRESETVALUE:
            if( n < OUTPUTCOUNT && parameter != NULL){
                if( *parameter ){
                    if( *parameter == '0' ){
                        config.ports[n].resetvalue = 0;
                    }else if( *parameter == '1' ){
                        config.ports[n].resetvalue =1;
                    }
                }
            }
            digitaloutput_save();
        break;
        case SET_OUTPUT:
            if( n < OUTPUTCOUNT && parameter != NULL){
                if( *parameter ){
                    if( *parameter == '0' ){
                        digitaloutput_setValue(n, 0);
                    }else if( *parameter == '1' ){
                        digitaloutput_setValue(n, 1);
                    }
                }
            }
        break;
    }
}

