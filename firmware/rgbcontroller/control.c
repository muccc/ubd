#include <stdint.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "fnordlicht.h"
#include "common.h"
#include "control.h"
#include "pwm.h"
#include "settings.h"
#include <string.h>
#include <avr/wdt.h>
#include "cmd_handler.h"
#include <avr/sleep.h>

uint16_t time = 0;
uint8_t oldr, oldg, oldb;
uint8_t oldtargetr, oldtargetg, oldtargetb;
uint16_t timeout;

void control_setColor(uint8_t r, uint8_t g, uint8_t b)
{
    cli();
    global_pwm.channels[0].brightness = r;
    global_pwm.channels[0].target_brightness = r;
    global_pwm.channels[1].brightness = g;
    global_pwm.channels[1].target_brightness = g;
    global_pwm.channels[2].brightness = b;
    global_pwm.channels[2].target_brightness = b;
    sei();
    global.state = STATE_REMOTE;
    global.nextstate = STATE_REMOTE;
}

void control_fade(uint8_t r, uint8_t g, uint8_t b, uint16_t speed)
{
    uint8_t pos;
    global_pwm.channels[0].target_brightness = r;
    global_pwm.channels[1].target_brightness = g;
    global_pwm.channels[2].target_brightness = b;
    for(pos = 0; pos < 3; pos++){
        global_pwm.channels[pos].speed_h = speed >> 8;
        global_pwm.channels[pos].speed_l = speed & 0xFF;
    }
    global.state = STATE_REMOTE;
    global.nextstate = STATE_REMOTE;
}

void control_fadems(uint8_t r, uint8_t g, uint8_t b, uint16_t time)
{
    uint8_t pos;
    uint8_t max = 0;
    uint8_t tmp;

    global_pwm.channels[0].target_brightness = r;
    global_pwm.channels[1].target_brightness = g;
    global_pwm.channels[2].target_brightness = b;

    for( pos = 0; pos < 3; pos++){
        tmp = abs(global_pwm.channels[pos].brightness -
                global_pwm.channels[pos].target_brightness);
        if( tmp > max ){
            max = tmp;
        }
    }
    //speed = steps/tick
    //time = steps / (speed * ticks_per_time)
    //speed = steps * 1000 / ( time * 144 );
    uint32_t lsteps = max * 1024L * 256L; //good enough
    uint16_t speed = lsteps / (time * PWM_STEPS_PER_SECOND);
    
    for(pos = 0; pos < 3; pos++){
        global_pwm.channels[pos].speed_h = speed >> 8;
        global_pwm.channels[pos].speed_l = speed & 0xFF;
    }
    //setting state manually since without STATE_REMOTE scripts will
    //inerfere with fadems. Because of this state has to be reset also
    ///manually when launching script. see => cmd_handler.c
    //TODO: add STATE_REMOTE handling to state-machine
    global.state = STATE_REMOTE;
    global.nextstate = STATE_REMOTE;
}

void control_fademsalt(uint8_t r, uint8_t g, uint8_t b, uint16_t time)
{
    uint8_t pos;
    global_pwm.channels[0].target_brightness = r;
    global_pwm.channels[1].target_brightness = g;
    global_pwm.channels[2].target_brightness = b;

    for(pos = 0; pos < 3; pos++){
        //about 1000 cycles per channel
        uint8_t a = global_pwm.channels[pos].brightness;
        uint8_t b = global_pwm.channels[pos].target_brightness;
        if( a == b ){
            continue;
        }
        uint8_t dist = abs(a-b);

        uint32_t lsteps = dist * 1024L * 256L; //good enough
        uint16_t speed = lsteps / (time * PWM_STEPS_PER_SECOND);
        global_pwm.channels[pos].speed_h = speed >> 8;
        global_pwm.channels[pos].speed_l = speed & 0xFF;
    }
    global.state = STATE_REMOTE;
    global.nextstate = STATE_REMOTE;
}

void control_flash(uint8_t r, uint8_t g, uint8_t b, uint16_t time)
{
    cli();
    oldtargetr = global_pwm.channels[0].target_brightness;
    oldtargetg = global_pwm.channels[1].target_brightness;
    oldtargetb = global_pwm.channels[2].target_brightness;
    
    oldr = global_pwm.channels[0].brightness;
    oldg = global_pwm.channels[1].brightness;
    oldb = global_pwm.channels[2].brightness;
    
    global_pwm.channels[0].brightness = r;
    global_pwm.channels[0].target_brightness = r;
    global_pwm.channels[1].brightness = g;
    global_pwm.channels[1].target_brightness = g;
    global_pwm.channels[2].brightness = b;
    global_pwm.channels[2].target_brightness = b;
    sei();

    timeout = time;    
    global.state = STATE_FLASH;
    global.nextstate = STATE_REMOTE;
}


void control_tick(void)
{
    switch(global.state){
        case STATE_REMOTE:
            control_faderunning = 1;
            global.flags.running = 0;
        break;
        case STATE_FLASH:
            if( timeout -- == 0 ){
                global.state = global.nextstate;
                cli();
                global_pwm.channels[0].brightness = oldr;
                global_pwm.channels[0].target_brightness = oldtargetr;
                global_pwm.channels[1].brightness = oldg;
                global_pwm.channels[1].target_brightness = oldtargetg;
                global_pwm.channels[2].brightness = oldb;
                global_pwm.channels[2].target_brightness = oldtargetb;
                sei();
            }
            break;
    }
}

