#include "TUM_Draw.h"

#include "FreeRTOS.h"
#include "semphr.h"

#ifndef TEST_MYSTRUCTS_H

#define TEST_MYSTRUCTS_H  

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

#define SCREEN_FREQUENCY    1000/60

#define PADDING         10

#define BULLET_HEIGHT   5
#define BULLET_WIDTH    2
#define BULLET_ACTIVE   1
#define BULLET_PASSIVE  2
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
#define ALIEN_WIDTH     30

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
    SemaphoreHandle_t lock;   
}alien_t;

typedef struct {
    spaceship_t *my_spaceship;
    bullet_t *my_bullet;
} game_objects_t;

game_objects_t *game_objects_init();

#endif /*TEST_MYSTRUCTS_H*/