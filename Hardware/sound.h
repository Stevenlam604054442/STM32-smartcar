#ifndef __sound_H
#define __sound_H

extern uint16_t Sound_Time; 
extern uint16_t distance; 
extern uint8_t get_distance_flag;
uint16_t sound_GetValue(void);
void sound_Start(void);
void sound_Init(void);

#endif
