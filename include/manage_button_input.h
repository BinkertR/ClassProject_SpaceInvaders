/*!
 * SpaceInvaders
 * @file manage_button_input.h
 * @author Roman Binkert
 * @date 19 June 2020
 * @brief the task the check which buttons are pressed and do the according things

*/


#ifndef TEST_MANAGEBUTTON_H

#define TEST_MANAGEBUTTON_H  

/**
 * @brief start the ManageButtonTask with a higher priority then all the other tasks (since this is managing all other tasks)
 * 
 * The ManageButtonTask does the following:
 *  - manages the game state and only check the available buttons in this state
 * 
 * The Game states are the following: 
 *  - Pre Game (Main Menu)
 *  - Cheat Menu
 *  - in Game
 *  - After Game
 *  - Pause Menu
*/
int MangageButtonInit(tasks_and_game_objects_t *tasks_and_game_objects);

#endif /*TEST_MANAGEBUTTON_H*/
