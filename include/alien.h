/*!
 * SpaceInvaders
 * @file alien.h
 * @author Roman Binkert
 * @date 20 June 2020
 * @brief the functions to draw the aliens on the screen and move them

*/


#include "TUM_Draw.h"

/**
 * @brief return the image handle to draw the alien according to the score.
 * Loads the img only if it hasn't been loaded already.
*/
image_handle_t AlienLoadImg(int alien_score);

/**
 * @brief draws all the active aliens of the alien matrix to the screen.
*/
int AlienDrawMatrix(game_objects_t *my_gameobjects);

/**
 * @brief starts the AlienCalcMatrix Task and returns the taskhandle.
 * 
 * The AlienCalcMatrixTask calculates:
 *  - the alien movement (based on level and killed aliens in this level)
 *  - if the aliens got hit by the players bullet
 *  - shoots the alien bullets randomly
 *  - initiates new level
 *  - check if alien reaches the bottom of the screen and if so end game
*/
TaskHandle_t AlienInitCalcMatrixTask(game_objects_t *my_game_objects);
