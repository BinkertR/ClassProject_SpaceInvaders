/*!
 * SpaceInvaders
 * @file mothership.h
 * @author Roman Binkert
 * @date 13 July 2020
 * @brief functions to display the mothership and communicate with the AI
*/

#include "TUM_Draw.h"
#include "my_structs.h"


#ifndef TEST_MOTHERSHIP_H

#define TEST_MOTHERSHIP_H  

int MothershipDraw(game_objects_t *game_objects);

image_handle_t MothershipLoadImg();

TaskHandle_t MotherShipInitCalcTask(game_objects_t *my_game_objects);

#endif /*TEST_MOTHERSHIP_H*/
