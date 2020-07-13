/*!
 * SpaceInvaders
 * @file my_structs.h
 * @author Roman Binkert
 * @date 19 June 2020
 * @brief the structures to store all the game data needed by the different tasks shared via pointers and Semaphores.

*/

#include "TUM_Draw.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#ifndef TEST_MYSTRUCTS_H

#define TEST_MYSTRUCTS_H  

#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

#define SCREEN_FREQUENCY    1000/60

#define OBJ_ACTIVE      1
#define OBJ_PASSIVE     0

#define PADDING         10

// game states
#define GAME_PRE_START  0
#define GAME_RUNNING    1
#define GAME_PAUSED     2
#define GAME_NEXT_LEVEL 3
#define GAME_ENDED      4
#define GAME_CHEAT_MENU 5

#define PLAYER_LIFES    2  // Player lifes besides the first one
#define LEVEL_SPEED_INCREASE_FAKTOR     2


// in game needed definitions

// bullet
#define BULLET_HEIGHT   5
#define BULLET_WIDTH    2
#define BULLET_SPEED    7

// spaceship
#define SHIP_HEIGHT     10
#define SHIP_WIDTH      50
#define GUN_HEIGHT      10
#define GUN_WIDHT       10
#define SHIP_SPEED      5
#define SHIP_X_MAX      SCREEN_WIDTH - PADDING- SHIP_WIDTH / 2
#define SHIP_X_MIN      PADDING + SHIP_WIDTH / 2
#define SHIP_Y_CO       SCREEN_HEIGHT - PADDING - SHIP_HEIGHT

// aliens
#define ALIEN_EASY      10
#define ALIEN_MIDDLE    20    
#define ALIEN_HARD      30
#define ALIENS_PER_ROW  8
#define ALIENS_PER_COLUMN   5
#define ALIEN_WIDTH     35
#define ALIEN_PADDING_X 25
#define ALIEN_PADDING_Y 10
#define ALIEN_START_X   50
#define ALIEN_START_Y   50
#define ALIEN_MIN_X     20
#define ALIEN_MAX_X     SCREEN_WIDTH - ALIEN_MIN_X
#define ALIEN_X_SPEED   0.3
#define ALIEN_ACCELERATION      0.05
#define ALIEN_Y_SPEED   5

// alien bullets
#define MAX_ACTIVE_ALIEN_BULLETS    10
#define ALIEN_BULLET_SPEED  3
#define RAND_SHOOT_CHANCE   1000  // 1 in RANDOM_SHOOT_CHANCE

// bunkers
#define NUMBER_OF_BUNKERS   4
#define BUNKER_CELL_SIZE    7
#define BUNKER_X_CELLS_NUMBER 10
#define BUNKER_POSITION_Y   SCREEN_HEIGHT - SHIP_HEIGHT - GUN_HEIGHT - 3 * BUNKER_CELL_SIZE - 3 * PADDING


typedef struct{
    coord_t position;  // position of the middle of the bullet
    int active;  // an integer to store if the bullet is currently active
    SemaphoreHandle_t lock;
}bullet_t;

typedef struct{
    bullet_t **first_bullet;  //pointer to the pointer of to the first of all possible alien bullets
    SemaphoreHandle_t lock;
}alien_bullet_t;


typedef struct{
    coord_t position;  // position of the middle of the spaceship
    int lifes;         // lifes the player / spaceship has left
    SemaphoreHandle_t lock;
}spaceship_t;

typedef struct {
    /* the float values are needed to keep track of movements smaller then one pixel per frame*/
    float x;       // float x value of something 
    float y;        // float y value of something
}coord_float_t;

typedef struct {
    coord_float_t position;  // using float to also keep track of position changes smaller then one pixel -> move every n screen ticks
    int alien_score;    // the score the player receives when killing this alien
    int active;         // an int to keep track if this alien is still active
    image_handle_t img_h;   // the img_handle which can be used by TUMDraw to draw the picture to the screen
    SemaphoreHandle_t lock;   
}alien_t;

typedef struct {
    alien_t **first_alien; // pointer to the pointer of the first alien in the column
    int lowest_active_alien;  // the number of the lowest(on screen) alien in this column (needed to see which alien can shoot).
    int *active_aliens;  //array with an entry for each alien. 1 if alien is active, else 0
    int active;         // int to keep track if this alien column is active
    SemaphoreHandle_t lock;
}alien_column_t;

typedef struct{
    alien_column_t **first_column;  
    int *active_columns;  //array with an entry for each column. 1 if column is active, else 0
    int leftest_active_column;  // the number of the most left column (needed to move the aliens always right to the side)
    int rightest_active_column;  // the number of the most right column (needed to move the aliens always right to the side)
    SemaphoreHandle_t lock;
}alien_matrix_t;

typedef struct{
    int current_score;
    int level;
    int lifes_left;
    int infitive_lifes;  // if 1 the player doesn't loose lives
    SemaphoreHandle_t lock;
}score_t;

typedef struct{
    int active;     // int to keep track if this bunker cell is still active (and should be displayed, used for calculations etc)
    coord_t position;  // the middle of the bunker cell. 
    int width;  // width and height of the bunker cell in pixels
}bunker_cell_t;


typedef struct{
    /*
    a bunker that has the following shape.
    each # stands for one cell that can be destroyed individually.

    #####
    #   #
    #####

    each row is represented as an array of cells. 
    The value of row is the pointer to the first element in this row.

    */
    coord_t position;  // the upper left corner of the bunker 
    bunker_cell_t *upper_row;
    bunker_cell_t *middle_row;  // has three inactive cells in the middle
    bunker_cell_t *lower_row;
    SemaphoreHandle_t lock;
}bunker_t;


typedef struct {
    /* a single object which holds pointers to all the other obejcts.
    
    is mainly used to be passed to the different tasks so that every task can access all the information.
    */
    spaceship_t *my_spaceship;
    bullet_t *my_bullet;
    bullet_t **alien_bullets;
    alien_matrix_t *alien_matrix;  //first_row->first_column = first_alien; starting at top left of screen
    bunker_t **bunkers;  // pointer to the first bunker
    score_t *score;
    SemaphoreHandle_t lock;
} game_objects_t;

typedef struct {
    /* used to store an array of all the game calculating tasks so they can be stopped/resumed if the game is stopped/resumed*/
    TaskHandle_t *tasks;  
    int length;
}taskhandle_array_t;

typedef struct {
    int game_state;
    int highscore;
    SemaphoreHandle_t lock;
}game_info_t;

typedef struct{
    /* a datatype to be passed to the manage button tasks so it has access to all the taskshandles and the game_objects*/
    game_objects_t *game_objects;
    game_info_t *game_info;
    taskhandle_array_t *game_task_handlers;
}tasks_and_game_objects_t;


alien_matrix_t *AlienInitMatrix();
int game_objects_init(game_objects_t *game_objects);
tasks_and_game_objects_t *tasks_and_game_objects_init();

#endif /*TEST_MYSTRUCTS_H*/