/*!
 * SpaceInvaders
 * @file alien.h
 * @author Roman Binkert
 * @date 20 June 2020
 * @brief the functions to draw the aliens on the screen and move them

*/


#include "TUM_Draw.h"

image_handle_t AlienLoadImg(int alien_score);

int AlienDrawMatrix(game_objects_t *my_gameobjects);

TaskHandle_t AlienInitCalcMatrixTask(game_objects_t *my_game_objects);
