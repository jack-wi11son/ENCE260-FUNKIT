// File:   ball.c
// Author: J Willson, S Hewitt
// Date:   11/10/2023
// Descr:  Module for ball related functions

#include "system.h"
#include "ball.h"
#include <stdbool.h>
#include "ir_uart.h"
#include <stdlib.h>




ball_t ball_create(uint8_t pos)
{
    ball_t ball;
    ball.active = true; //true if on screen
    ball.pos = pos;
    ball.dist = 1; //spawns one row in front of player
    ball.send = true; //away from player
    ball.ticks = 0; //timer for moving balls at different rates
    return ball;
}

void ball_update(ball_t* balls, uint8_t* num_balls, uint16_t fire_rate, uint8_t* player_pos, uint8_t* lives)
{
    for (int i = 0; i < *num_balls; i++) {
        if (balls[i].active) {
            if (balls[i].ticks > fire_rate) {
                balls[i].ticks = 0;
                if (balls[i].send) { //if ball moving away from player
                    if (balls[i].dist < 4) {
                        balls[i].dist++;
                    } else {
                        balls[i].active = false;
                        ball_send(balls[i].pos);

                    }
                } else {
                    if ((balls[i].dist < 1) && (balls[i].pos == *player_pos)) { //if player hit
                        (*lives)--;
                        balls[i].active = false;
                    } else if (balls[i].dist > 0) {
                        balls[i].dist--; //move ball towards enemy
                    } else {
                        balls[i].active = false; //remove ball
                    }
                }
            } else {
                balls[i].ticks++; //increment timer
            }
        }
    }
}


void ball_send(uint8_t pos)
{
    ir_uart_putc(pos + '0');
}


ball_t ball_recieve(char recieved_ball_char) 
{
    ball_t ball = {true, ('6' - recieved_ball_char), 4, false, 0};
    return ball;
}

uint8_t ball_arr_clean(ball_t* balls, uint8_t num_balls, uint8_t MAX_BALLS) //Deletes inactive balls in array after certain length
{
    ball_t* ballstemp = calloc(MAX_BALLS, sizeof(ball_t));
    uint8_t new_num_balls = 0;
    for (int i = 0; i < num_balls; i++) {
        if (balls[i].active) {
            ballstemp[new_num_balls] = balls[i];
            new_num_balls++;
        }
    }
    free(balls);
    balls = ballstemp; //Points to new array
    return new_num_balls;
}