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

/**
 * @brief draw the mothership to the screen depending on its current position
*/
int MothershipDraw(game_objects_t *game_objects);

/**
 * @brief load the mothership img, scale it and return the image_handle to it
*/
image_handle_t MothershipLoadImg();

/**
 * @brief starts the MotherShipAITask and returns its handle.
 * 
 * The MotherShipAITask does the following:
 *  - opens a UDP Socket to communicate with the opponent binary
 *  - if the socekt receives something adapt the mothership direction so it can be moved in the right direction
 *  - send PAUSE/RESUME if the mothership was set to passive/active
 *  - send diff, ATTACKING/PASSIVE to opponent binary
 *  - move the mothership according to the current direction
*/
TaskHandle_t MothershipInitAITask(tasks_and_game_objects_t *tasks_and_game_objects);


/**
 * @brief starts the MotherShipCalcTask (for single player) and returns its handle
 * 
 * The MotherShipCalcTask does the following:
 *  - if the mothership is passive give it a random chance to get active
 *  - if the mothership is active move it from the left to the right of the screen 
 *  - if the mothership moved out of the right of the screen: set it to passive
*/
TaskHandle_t MotherShipInitCalcTask(game_objects_t *game_objects);

#endif /*TEST_MOTHERSHIP_H*/
