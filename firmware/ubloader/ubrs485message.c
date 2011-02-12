#include "ubrs485message.h"

uint8_t rs485msg_message[UB_PACKETLEN];
uint8_t rs485msg_len = 0;
uint8_t rs485msg_type = UB_NONE;

void rs485msg_init(void)
{
    rs485msg_len = 0;
}

//returns the message type or UB_NONE
uint8_t rs485msg_put(uint8_t data)
{
    static uint8_t escaped = 0;
    static uint8_t msgtype = UB_NONE;

    //tx(data);
    if( data == UB_ESCAPE && escaped == 0 ){
        //a control code will follow
        escaped = 1;
        return UB_NONE;
    }else if( escaped ){
        escaped = 0;
        if(data == UB_START || data == UB_QUERY){
            msgtype = data;
            rs485msg_len = 0;
            return UB_NONE;
        }else if( data == UB_DISCOVER ){
            return rs485msg_type = data;
        }else if( data == UB_BOOTLOADER ){
            msgtype = data;
            rs485msg_len = 0;
            //tx('B');
            return UB_NONE;
        }else if( data == UB_STOP ){
            //tx('S');
            if( msgtype == UB_START ||
                msgtype == UB_BOOTLOADER ){
                rs485msg_type = msgtype;
                msgtype = UB_NONE;
                return rs485msg_type;
            }else{
                msgtype = UB_NONE;
                return UB_NONE;
            }
        }
    }
    rs485msg_message[rs485msg_len++] = data;

    //prevent a buffer overflow
    if( rs485msg_len == UB_PACKETLEN )
        rs485msg_len--;

    //return only this single byte
    if( msgtype == UB_QUERY ){
        msgtype = UB_NONE; 
        return rs485msg_type = UB_QUERY;
    }
    return UB_NONE;
}

inline uint8_t * rs485msg_getMsg(void)
{
    
    return rs485msg_message;
}

inline uint8_t rs485msg_getLen(void)
{
    return rs485msg_len;
}

inline uint8_t rs485msg_getType(void)
{
    return rs485msg_type;
}
