#ifndef BALL_H
#define BALL_H

#include "system.h"


typedef struct { //struct for representing ball
    bool active;
    uint8_t pos;
    int8_t dist;
    bool send;
    uint8_t ticks;
} ball_t;

ball_t ball_create(uint8_t pos);
void ball_update(ball_t* balls, uint8_t* num_balls, uint16_t fire_rate, uint8_t* player_pos, uint8_t* lives);
ball_t ball_recieve(char recieved_ball_char);
void ball_send(uint8_t pos);
uint8_t ball_arr_clean(ball_t* balls, uint8_t num_balls, uint8_t MAX_BALLS);


#endif