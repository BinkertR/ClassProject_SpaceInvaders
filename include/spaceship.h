#include "TUM_Draw.h"

#ifndef TEST_SPACESHIP_H

#define TEST_SPACESHIP_H  

typedef struct{
    int x_position;
    int lifes;
    SemaphoreHandle_t lock;
}spaceship_t;



spaceship_t *SpaceShipInit();

int SpaceShipDraw(spaceship_t *my_spaceship);

int SpaceShipMoveRight(spaceship_t *my_spaceship);

int SpaceShipMoveLeft(spaceship_t *my_spaceship);

#endif /*TEST_SPACESHIP_H*/