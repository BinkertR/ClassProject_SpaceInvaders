/*!
 * SpaceInvaders
 * @file spaceship.h
 * @author Roman Binkert
 * @date 19 June 2020
 * @brief functions to display the spaceship and move it
*/

#include "FreeRTOS.h"
#include "semphr.h"

#include "TUM_Draw.h"

#ifndef TEST_SPACESHIP_H

#define TEST_SPACESHIP_H  

/**
 * @brief draw the spaceship with gun on its current position
*/
int SpaceShipDraw(spaceship_t *my_spaceship);

/**
 * @brief move the spaceship to the right according to its speed (should be called is right arrow is pressed)
*/
int SpaceShipMoveRight(spaceship_t *my_spaceship);

/**
 * @brief move the spaceship to the left according to its speed (should be called is left arrow is pressed)
*/
int SpaceShipMoveLeft(spaceship_t *my_spaceship);

#endif /*TEST_SPACESHIP_H*/