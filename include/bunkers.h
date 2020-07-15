/*!
 * SpaceInvaders
 * @file bunkers.h
 * @author Roman Binkert
 * @date 11 July 2020
 * @brief the functions to draw the bunkers and the calculating task to manage them

*/

#ifndef TEST_BUNKERS_H

#define TEST_BUNKERS_H  

#include "my_structs.h"

/**
 * @brief draw all the bunkers consisting out of multiple active bunker cells
*/
int BunkersDraw(game_objects_t *gameobjects);

/**
 * @brief starts the BunkersManage Task and returns the taskhandle.
 * 
 * The BunkersManage Task does the following: 
 *  - for all bunkers: 
 *      - check if one of it cells got hit by either a spaceship bullet or a alien bullet.
 *      - if a cell got hit: destroy the cell and the bullet from which it got hit.
*/
TaskHandle_t BunkersInitManageTask(game_objects_t *my_game_objects);

#endif // TEST_BUNKERS_H