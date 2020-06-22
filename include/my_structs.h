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

#define PADDING         10

#define BULLET_HEIGHT   5
#define BULLET_WIDTH    2
#define OBJ_ACTIVE      1
#define OBJ_PASSIVE     0
#define BULLET_SPEED    5

#define SHIP_HEIGHT     10
#define SHIP_WIDTH      50
#define GUN_HEIGHT      10
#define GUN_WIDHT       10
#define SHIP_SPEED      5
#define SHIP_X_MAX      SCREEN_WIDTH - PADDING- SHIP_WIDTH / 2
#define SHIP_X_MIN      PADDING + SHIP_WIDTH / 2
#define SHIP_Y_CO       SCREEN_HEIGHT - PADDING - SHIP_HEIGHT

#define PLAYER_LIFES    2

#define ALIEN_EASY      10
#define ALIEN_MIDDLE    20    
#define ALIEN_HARD      30
#define ALIENS_PER_ROW  8
#define ALIENS_PER_COLUMN   5
#define ALIEN_WIDTH     40
#define ALIEN_PADDING_X 25
#define ALIEN_PADDING_Y 10
#define ALIEN_START_X   50
#define ALIEN_START_Y   50
#define ALIEN_MIN_X     20
#define ALIEN_MAX_X     SCREEN_WIDTH - ALIEN_MIN_X
#define ALIEN_X_SPEED   1
#define ALIEN_Y_SPEED   1

typedef struct{
    coord_t position;
    int active;
    SemaphoreHandle_t lock;
}bullet_t;

typedef struct{
    int x_position;
    int lifes;
    SemaphoreHandle_t lock;
}spaceship_t;

typedef struct {
    coord_t position;
    int alien_score;
    int active;
    image_handle_t img_h;
    SemaphoreHandle_t lock;   
}alien_t;

typedef struct {
    alien_t **first_alien; // pointer to the pointer of the first alien in the column
    int lowest_active_alien;  // the number of the lowest(on screen) alien in this column (needed to see which alien can shoot).
    int *active_aliens;  //array with an entry for each alien. 1 if alien is active, else 0
    int active;
    SemaphoreHandle_t lock;
}alien_column_t;

typedef struct{
    alien_column_t **first_column;  
    int *active_columns;  //array with an entry for each column. 1 if column is active, else 0
    int leftest_active_column;  // the number of the most left column (needed to move the aliens always right to the side)
    int rightest_active_column;  // the number of the most right column (needed to move the aliens always right to the side)
}alien_row_t;



typedef struct {
    spaceship_t *my_spaceship;
    bullet_t *my_bullet;
    alien_row_t *alien_matrix;  //first_row->first_column = first_alien; starting at top left of screen
} game_objects_t;

game_objects_t *game_objects_init();

#endif /*TEST_MYSTRUCTS_H*/