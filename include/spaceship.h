#include "FreeRTOS.h"
#include "semphr.h"

#include "TUM_Draw.h"

#ifndef TEST_SPACESHIP_H

#define TEST_SPACESHIP_H  

int SpaceShipDraw(spaceship_t *my_spaceship);

int SpaceShipMoveRight(spaceship_t *my_spaceship);

int SpaceShipMoveLeft(spaceship_t *my_spaceship);

#endif /*TEST_SPACESHIP_H*/