/*!
 * SpaceInvaders
 * @file manage_screen.h
 * @author Roman Binkert
 * @date 19 June 2020
 * @brief the task to draw the current active objects/menu to the screen

*/

#ifndef TEST_MANAGESCREEN_H

#define TEST_MANAGESCREEN_H  

/**
 * @brief start the manage screen task and return its handle
 * 
 * The manage screen task does the following:
 *  - displays the menu depending on the game state
 *  - displays all the active game objects if the game is running
 * 
*/
TaskHandle_t ManageScreenInit(tasks_and_game_objects_t *tasks_and_game_objects);

#endif /*TEST_MANAGESCREEN_H*/
