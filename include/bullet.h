/*!
 * SpaceInvaders
 * @file bullet.h
 * @author Roman Binkert
 * @date 19 June 2020
 * @brief the functions to draw bullets and calculate them

*/

#include "FreeRTOS.h"
#include "semphr.h"
#include "TUM_Draw.h"

#ifndef TEST_BULLET_H

#define TEST_BULLET_H  

#include "my_structs.h"


/**
 * @brief shot the spaceship bullet out of the middle of the curernt gut position if the player has no active bullet already 
*/
int BulletShoot(spaceship_t *my_spaceship, bullet_t *my_bullet);

/**
 * @brief draw all the active bulltes shot by the aliens
 * 
*/
int BulletAlienDraw(game_objects_t *gameobjects);

/**
 * @brief draw the spaceship bullet if its active
*/
int BulletDraw(bullet_t *my_bullet);

/**
 *  @brief starts the BulletCalcTask and returns the taskhandle.
 * 
 * The BulletCalcTask calculates the following:
 *  - move spaceship bullet
 *  - check if mothership got hit by spaceship bullet and if so destroy mothership
 *  - check if spaceship bullet is out of screen and if so destroy it
 * 
 *  - for every active alien bullet:
 *      - move alien bullet
 *      - check if it hits the spaceship and if so subtract one life (if infinite lifes is turned off)
 *      - check if it is hit by the spaceship bullet and if so destroy both
 *      - check if the bullet is out of the screen and if so destroy it
*/
TaskHandle_t BulletInitCalcTask(game_objects_t *my_game_objects);

#endif /*TEST_BULLET_H*/