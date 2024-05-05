#ifndef PLAYER_H
#define PLAYER_H

#include "system.h"

typedef struct {
    uint8_t pos;
    uint8_t lives;
} player_t;


void player_init(player_t* player);  // Initialise player params

void player_left(player_t* player);  //Move left

void player_right(player_t* player); //Move right

#endif