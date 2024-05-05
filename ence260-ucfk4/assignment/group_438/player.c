// File:   player.c
// Author: J Willson, S Hewitt
// Date:   11/10/2023
// Descr:  Module for player related functions

#include "system.h"

#include "player.h"

// Initialize the player with a default position and lives
void player_init(player_t* player)
{
    player->pos = 3; // Start in the middle of the screen
    player->lives = 3; // Set the initial number of lives to 3
}

// Move the player to the left (increment position) if not already at the leftmost position
void player_left(player_t* player)
{
    if (player->pos != 6) {
        player->pos++;
    }
}

// Move the player to the right (decrement position) if not already at the rightmost position
void player_right(player_t* player)
{
    if (player->pos != 0) {
        player->pos--;
    }
}
