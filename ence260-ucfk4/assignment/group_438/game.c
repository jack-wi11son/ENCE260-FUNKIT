// File:   game.c
// Author: J Willson, S Hewitt
// Date:   11/10/2023
// Descr:  Main game file for the game

#include "system.h"
#include "tinygl.h"
#include "navswitch.h"
#include "ir_uart.h"
#include <stdlib.h>
#include <stdbool.h>
#include "pacer.h"
#include "../fonts/font3x5_1.h"

#include "player.h"
#include "ball.h"


/* Define polling rates in Hz. */
#define DISPLAY_TASK_RATE 250      /* Display rate */
#define NAVSWITCH_TASK_RATE 250    /* Navswitch rate */
#define GAME_TASK_RATE 250         /* Game rate */

#define PACER_RATE 500             /* Pacer loop - controls main loop */
#define MESSAGE_RATE 20            /* Text scroll rate */
#define PIXEL_ON true              /* Pixel turned on */
#define GUN_COL 4                  /* Column number for the gun */
#define MAX_BALLS 50               /* Maximum number of balls allowed in array */
#define LEVEL_1_SPEED 50           /* Level 1: Fire rate (in PACER_RATE cycles) */
#define LEVEL_2_SPEED 25           /* Level 2: Fire rate (in PACER_RATE cycles) */
#define LEVEL_3_SPEED 10           /* Level 3: Fire rate (in PACER_RATE cycles) */
#define START_MSG " PRESS TO START "  /* Start message */
#define LOSS_MSG " YOU LOST :( PRESS " /* Loss message */
#define WON_MSG " YOU WON :) PRESS "   /* Win message */

typedef enum {
    STATE_START,       /* Game starting state */
    STATE_PLAYING,     /* Game playing state */
    STATE_OVER,        /* Game over state */
    STATE_READY,       /* Ready state (not used yet) */
    STATE_MENU_LEVEL   /* Level selection state */
} state_t;

static state_t state = STATE_START;   /* Current game state */
static player_t* player;            /* Player data structure pointer */
static ball_t* balls;              /* Array of ball data structures */
static char LEVEL = '1';            /* Current game level (default: level 1) */
static uint8_t num_balls = 0;      /* Number of active balls on the screen */
static uint16_t fire_rate = 0;     /* Ball movement speed (depends on the level selected) */

static char recieve_ir_level = '!'; /* Received level via IR communication ('!' when random character received)*/
static char received_char;          /* Received character via IR communication */

/* Function to allocate memory for player and balls */
void allocate_mem(void) {
    ball_t* ballstemp = calloc(MAX_BALLS, sizeof(ball_t));  // Create array of balls, MAX_BALLS long
    balls = ballstemp;
    player_t* playertemp = calloc(1, sizeof(player_t)); // Allocate memory for the player struct
    player = playertemp;
    num_balls = 0;
}

/* Function to increase the game level */
void inc_level(void) {
    if (LEVEL != '3') {
        LEVEL++;
    }
}

/* Function to decrease the game level */
void dec_level(void) {
    if (LEVEL != '1') {
        LEVEL--;
    }
}

/* Function to set the fire rate based on the current game level */
void set_fire_rate(void) {
    switch (LEVEL) {
        case '1':
            fire_rate = LEVEL_1_SPEED; // Slow
            break;
        case '2':
            fire_rate = LEVEL_2_SPEED; // Medium
            break;
        case '3':
            fire_rate = LEVEL_3_SPEED; // Fast
            break;
        default:
            break;
    }
}

/* Function to display scrolling text on the LED matrix */
static void display_text(const char str[]) {
    tinygl_font_set(&font3x5_1);                        //Smaller sideways text
    tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
    tinygl_text_speed_set(MESSAGE_RATE);
    tinygl_text_dir_set(TINYGL_TEXT_DIR_ROTATE);
    tinygl_text(str);
}

/* Function to display a single character on the LED matrix */
void display_character(char character) {
    tinygl_clear();
    tinygl_point_t pos = {0, 4};            //Display in the center
    tinygl_draw_char(character, pos);
}

/* Function to check for the level received via IR communication */
char check_level_sent(void) {
    if (ir_uart_read_ready_p()) {
        recieve_ir_level = ir_uart_getc();
        if ((recieve_ir_level >= '1') && (recieve_ir_level <= '3')) {
            return recieve_ir_level;
        }
    }
    return '!';    // Wrong character received
}

/* Function to display the player's position on the LED matrix */
void display_player(player_t* player) {
    display_pixel_set(GUN_COL, player->pos, PIXEL_ON);
}

/* Function to display the balls on the LED matrix */
void display_balls(void) {
    for (int i = 0; i < num_balls; i++) {
        if (balls[i].active) {
            display_pixel_set(GUN_COL - balls[i].dist, balls[i].pos, PIXEL_ON);
        }
    }
}

/* Function for the display task, responsible for updating the LED matrix */
static void display_task(void) {
    switch (state) {
        case STATE_START:
            break;
        case STATE_MENU_LEVEL:
            display_character(LEVEL); // Display characters for levels
            recieve_ir_level = check_level_sent();  // Check if other player has selected a level
            if (recieve_ir_level != '!') { // If level charater received
                LEVEL = recieve_ir_level;
                tinygl_clear();
                player_init(player);   //Initialise player
                state = STATE_PLAYING; //Start playing
            }
            set_fire_rate();  // Set ball speed
            break;
        case STATE_PLAYING:
            display_clear();
            display_player(player);  // Display player pixel
            display_balls();         // Display all active ball pixels
            break;
        case STATE_OVER:
            break;
        default:
            break;
    }

    tinygl_update();  // Updates display constantly
}


/* Function for the navigation switch task */
static void navswitch_task(void) {
    navswitch_update(); // Check the status of the navigation switch

    if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
        // Handle push event based on the current game state
        switch (state) {
            case STATE_START:
                tinygl_clear(); // Clear the display
                state = STATE_MENU_LEVEL; // Transition to the level selection menu
                break;
            case STATE_MENU_LEVEL:
                tinygl_clear(); // Clear the display
                player_init(player); // Initialize the player
                state = STATE_PLAYING; // Transition to the playing state
                ir_uart_putc(LEVEL); // Inform the other board of the selected level and to start
                break;
            case STATE_PLAYING:
                balls[num_balls] = ball_create(player->pos); // Create a new ball
                num_balls++;
                break;
            case STATE_OVER:
                tinygl_clear(); // Clear the display
                display_text(START_MSG); // Display the start message
                free(player); // Free memory associated with the player
                free(balls); // Free memory associated with the balls
                allocate_mem(); // Reallocate memory
                state = STATE_START; // Transition to the start state
                break;
            default:
                break;
        }
    }

    if (navswitch_push_event_p(NAVSWITCH_SOUTH)) {
        // Handle south button press event based on the current game state
        switch (state) {
            case STATE_START:
                break;
            case STATE_MENU_LEVEL:
                dec_level(); // Decrease the game level
                break;
            case STATE_PLAYING:
                player_left(player); // Move the player to the left
                break;
            default:
                break;
        }
    }

    if (navswitch_push_event_p(NAVSWITCH_NORTH)) {
        // Handle north button press event based on the current game state
        switch (state) {
            case STATE_START:
                break;
            case STATE_MENU_LEVEL:
                inc_level(); // Increase the game level
                break;
            case STATE_PLAYING:
                player_right(player); // Move the player to the right
                break;
            default:
                break;
        }
    }
}

/* Function for the game task, responsible for updating the game state */
static void game_task(void) {
    switch (state) {
        case STATE_START:
            break;
        case STATE_MENU_LEVEL:
            break;
        case STATE_PLAYING:
            if (ir_uart_read_ready_p()) {
                received_char = ir_uart_getc();
                if ((received_char >= '0') && (received_char <= '6')) {
                    balls[num_balls] = ball_recieve(received_char); // Create a ball based on received data
                    num_balls++;
                } else if (received_char == 'G') {
                    tinygl_clear(); // Clear the display
                    display_text(WON_MSG); // Display the win message
                    state = STATE_OVER; // Transition to the game over state
                }
            }
            if (state == STATE_PLAYING) {
                ball_update(balls, &num_balls, fire_rate, &(player->pos), &(player->lives)); // Update ball positions and check for collisions
                if (player->lives == 0) {
                    tinygl_clear(); // Clear the display
                    display_text(LOSS_MSG); // Display the loss message
                    ir_uart_putc('G'); // Inform the other board of the game over
                    state = STATE_OVER; // Transition to the game over state
                }
            }
            if (num_balls >= MAX_BALLS) {
                // Handle the case when the maximum number of balls is reached
                // num_balls = ball_arr_clean(balls, num_balls, MAX_BALLS);
            }
            break;
        case STATE_OVER:
            break;
        default:
            break;
    }
}

int main(void) {
    // Initialize the system, pacer, display, navigation switch, and IR communication
    system_init();
    pacer_init(PACER_RATE);
    tinygl_init(DISPLAY_TASK_RATE);
    navswitch_init();
    ir_uart_init();

    display_text(START_MSG); // Display the start message
    allocate_mem(); // Allocate memory for player and balls

    uint16_t display_tick = 0;
    uint16_t navswitch_tick = 0;
    uint16_t game_tick = 0;

    state = STATE_START; // Set the initial game state to start

    while (1) {
        pacer_wait();

        // Update the display, navigation switch, and game tasks based on their respective rates
        if (display_tick >= (PACER_RATE / DISPLAY_TASK_RATE)) {
            display_tick = 0;
            display_task();
        }
        if (navswitch_tick >= (PACER_RATE / NAVSWITCH_TASK_RATE)) {
            navswitch_tick = 0;
            navswitch_task();
        }
        if (game_tick >= (PACER_RATE / GAME_TASK_RATE)) {
            game_tick = 0;
            game_task();
        }

        display_tick++;
        navswitch_tick++;
        game_tick++;
    }

    free(player); // Free memory associated with the player
    free(balls); // Free memory associated with the balls
    return 0;
}
