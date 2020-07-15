/*!
 * SpaceInvaders
 * @file my_structs.h
 * @author Roman Binkert
 * @date 19 June 2020
 * @brief the structures and defines create all the game objects needed by the different tasks shared via pointers and Semaphores.
 * 
 * 
 * 
 * All of these structures are initilazied with the corresponding functions in my_structs.c

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

#define PLAYMODE_SINGEPLAYER    1
#define PLAYMODE_AI_PLAYER      0  // needs to be 0 and 1 because this is also used as index for array

#define GAME_GAME_TASKS_AMOUNT  3
#define GAME_PLAYMODE_TAKS_AMOUNT 2

#define PLAYER_LIFES    2  // Player lifes besides the first one
#define LIFE_BACK_EVERY_N_LEVELS        2       // the player gets one life back when the next level % this == 0
#define LEVEL_SPEED_INCREASE_FAKTOR     2  // how much faster the aliens should move at the beginning of each level


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

// mothership
#define MOTHERSHIP_WIDTH 40
#define MOTHERSHIP_Y_CO PADDING * 5
#define MOTHERSHIP_SPEED 2
#define MOTHERSHIP_HEIGHT 20
#define MOTHERSHIP_SCORE  400
#define MOTHERHSIP_APPERANCE_CHANCE 3000  // 1 in MOTHERHSIP_APPERANCE_CHANCE

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
#define ALIEN_START_Y   MOTHERSHIP_Y_CO + MOTHERSHIP_HEIGHT + PADDING
#define ALIEN_MIN_X     20
#define ALIEN_MAX_X     SCREEN_WIDTH - ALIEN_MIN_X
#define ALIEN_X_SPEED   0.05
#define ALIEN_ACCELERATION      0.2   // how much faster the aliens should move per alien that was killed
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

/**
 * @brief object to create the spaceshipbullet that can destroy objects (mothership, aliens, alien bullets)
 * 
 * the bullets can be shot by the spaceship when the player presses space and has no active bullet.
 * 
 * 
*/
typedef struct{
    coord_t position;  // position of the middle of the bullet
    int active;  // an integer to store if the bullet is currently active
    SemaphoreHandle_t lock;
}bullet_t;


/**
 * @brief a struct to safe all the active alien bullets (max number of MAX_ACTIVE_ALIEN_BULLETS) in an array.
 * 
 * The alien bullets are shot randomly by the lowest alien of one column. 
 * The alein bullet gets shot 1 in every RAND_SHOOT_CHANCE times the alien is checked.
 * Therefor the screenfrequency is really important.
 * 
*/
typedef struct{
    bullet_t **first_bullet;  //pointer to the pointer of to the first of all possible alien bullets
    SemaphoreHandle_t lock;
}alien_bullet_t;


/** 
 * @brief a struct to create the spaceship controlled by the player. 
 * 
*/
typedef struct{
    coord_t position;  // position of the middle of the spaceship
    SemaphoreHandle_t lock;
}spaceship_t;

/**
 * @brief a struct so create the mothership controlled by either AI (in multiplayer mode) or random apprerance change (singleplayer mode).
 * 
*/
typedef struct{
    coord_t position;  // position of the middle of the mothership
    image_handle_t img_h;   // the img_handle which can be used by TUMDraw to draw the picture to the screen
    int active;  // keep track if the mothership is currently active (used to send PAUSE/RESUME)
    int direction; // used to store INC/DEC/HALT in AI Mode
    SemaphoreHandle_t lock;
}mothership_t;

 /**
 * @brief the float values are needed to keep track of movements smaller then one pixel per frame
 * 
 * mostly used for alien positions
 * 
 * */
typedef struct {   
    float x;       // float x value of something 
    float y;        // float y value of something
}coord_float_t;

/**
 * @brief a struct to create a single alien which is part of the alien matrix and therefor also controlled by the alien matrix.
*/
typedef struct {
    coord_float_t position;  // using float to also keep track of position changes smaller then one pixel -> move every n screen ticks
    int alien_score;    // the score the player receives when killing this alien
    int active;         // an int to keep track if this alien is still active
    image_handle_t img_h;   // the img_handle which can be used by TUMDraw to draw the picture to the screen
    SemaphoreHandle_t lock;   
}alien_t;

/**
 * @brief a struct holding a hole column of aliens of the alien matrix.
 * 
 * One column consist of one high reward alien at the top, followed by two middle reward aliens and two low reward aliens.
 * 
*/
typedef struct {
    alien_t **first_alien; // pointer to the pointer of the first alien in the column
    int lowest_active_alien;  // the number of the lowest(on screen) alien in this column (needed to see which alien can shoot).
    int *active_aliens;  //array with an entry for each alien. 1 if alien is active, else 0
    int active;         // int to keep track if this alien column is active
    SemaphoreHandle_t lock;
}alien_column_t;

/**
 * @brief a struct to hold all the aliens of the alien matrix sortet by column
 * 
 * this struct is used to move all the aliens 
 * 
*/
typedef struct{
    alien_column_t **first_column;  
    int *active_columns;  //array with an entry for each column. 1 if column is active, else 0
    int leftest_active_column;  // the number of the most left column (needed to move the aliens always right to the side)
    int rightest_active_column;  // the number of the most right column (needed to move the aliens always right to the side)
    float current_alien_x_speed;
    int killed_aliens_in_stage;  // used to calc the alien speed factor by killey aliens in this stage
    SemaphoreHandle_t lock;
}alien_matrix_t;


/**
 * @brief a cell of a bunker which can be destroyed by a bullet hit
*/
typedef struct{
    int active;     // int to keep track if this bunker cell is still active (and should be displayed, used for calculations etc)
    coord_t position;  // the middle of the bunker cell. 
    int width;  // width and height of the bunker cell in pixels
}bunker_cell_t;

/**
 * @brief a bunker consisting out of multiple cells where behind the player can hide from alien bullets. It is destroyed incrementally by a hit of alien bullets
 * 
 * a bunker that has the following shape.
 * each # stands for one cell that can be destroyed individually.
 *
 *   #####
 *   #   #
 *   #####
 *
 * each row is represented as an array of cells. 
 * The value of row is the pointer to the first element in this row.
 *
*/
typedef struct{
    coord_t position;  // the upper left corner of the bunker 
    bunker_cell_t *upper_row;
    bunker_cell_t *middle_row;  // has three inactive cells in the middle
    bunker_cell_t *lower_row;
    SemaphoreHandle_t lock;
}bunker_t;

/**
 * @brief a struct to keep track of the current game state 
 * 
 * all of these values have to be reset when a new game is started.
 * 
*/
typedef struct{
    int current_score;  // the score reached in the current game
    int level;          // the level reached in the current game
    int lifes_left;     // the lifes the player has left (besides the active one)
    int infitive_lifes;  // cheat: if 1 the player doesn't loose lives
    SemaphoreHandle_t lock;
}score_t;



/**
 * @brief a object holding the pointers to all the game specific important data.
 * 
 * this object is passed to the tasks so they can access the gamedata.
 * 
 * all of these values have to be reset if the game is restarted.
*/
typedef struct {
    spaceship_t *my_spaceship;
    mothership_t *mothership;
    bullet_t *my_bullet;
    bullet_t **alien_bullets;
    alien_matrix_t *alien_matrix;  //first_row->first_column = first_alien; starting at top left of screen
    bunker_t **bunkers;  // pointer to the first bunker
    score_t *score;
    SemaphoreHandle_t lock;
} game_objects_t;


/**
 * @brief used to store an array of all the task_handles s
 */
typedef struct {
    TaskHandle_t *tasks;  
    int length;
}taskhandle_array_t;

/**
 * @brief the gamevalues that have to be stored during the hole game session
*/
typedef struct {
    int game_state; // menu where the user is in (pregame, in game, pause, end game, ...)
    int highscore;
    int playmode;  // single or multiplayer (mothership AI or not)
    SemaphoreHandle_t lock;
}game_info_t;

/**
 * @brief
*/
typedef struct{
    /* a datatype to be passed to the manage button tasks so it has access to all the taskshandles and the game_objects*/
    game_objects_t *game_objects;
    game_info_t *game_info;
    taskhandle_array_t *game_task_handlers;
    taskhandle_array_t *playmode_task_handlers;
}tasks_and_game_objects_t;


alien_matrix_t *AlienInitMatrix();  // init the alien matrix. Used by alienTask to reload aliens after reaching a new level
int game_objects_init(game_objects_t *game_objects);  // used to reset the game (used by button task to restart the game)
tasks_and_game_objects_t *tasks_and_game_objects_init();  // used for the first initalization in main.c

#endif /*TEST_MYSTRUCTS_H*/