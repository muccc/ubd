#ifndef __CONTROL_H_
#define __CONTROL_H_

#define STATE_RUNNING       0
#define STATE_STANDBY       1
#define STATE_PAUSE         2
#define STATE_ENTERSTANDBY  3
#define STATE_LEAVESTANDBY  4
#define STATE_ENTERSLEEP    5
#define STATE_SLEEP         6
#define STATE_REMOTE        7
#define STATE_LOWBAT        8
#define STATE_ENTERPOWERDOWN    9
#define STATE_FLASH         10
#define SLEEP_TIME          (180 * 125)        //125 equals 1 second

extern uint8_t control_faderunning;

void control_init(void);
void control_setColor(uint8_t r, uint8_t g, uint8_t b);
void control_fade(uint8_t r, uint8_t g, uint8_t b, uint16_t speed);
void control_fadems(uint8_t r, uint8_t g, uint8_t b, uint16_t time);
void control_flash(uint8_t r, uint8_t g, uint8_t b, uint16_t time);
void control_fademsalt(uint8_t r, uint8_t g, uint8_t b, uint16_t time);
void control_setTimeout(void);
void control_tick(void);
void control_standby(uint16_t wait);
void control_lowbat(void);
#endif
