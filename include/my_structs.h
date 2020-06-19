#include "TUM_Draw.h"

#include "FreeRTOS.h"
#include "semphr.h"

#ifndef TEST_MYSTRUCTS_H

#define TEST_MYSTRUCTS_H  

#define BULLET_HEIGHT   5
#define BULLET_WIDTH    2
#define BULLET_ACTIVE   1
#define BULLET_PASSIVE 2

#define SHIP_HEIGHT     10
#define SHIP_WIDTH      50
#define GUN_HEIGHT      10
#define GUN_WIDHT       10
#define SHIP_SPEED      5

#define PLAYER_LIFES    2

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
    spaceship_t *my_spaceship;
    bullet_t *my_bullet;
} game_objects_t;

game_objects_t *game_objects_init();

#endif /*TEST_MYSTRUCTS_H*/