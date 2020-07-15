#include <stddef.h>
#include <time.h>
#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Event.h"

#include "my_structs.h"
#include "spaceship.h"
#include "bullet.h"


#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

#define DEBOUNCE_DELAY 50  // in ms

TaskHandle_t ManageButtonTask = NULL;

typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
} buttons_buffer_t;

static buttons_buffer_t buttons = { 0 };

typedef struct count_button {
    // struct used for task 3.2.3 to read and write from different tasks
        SemaphoreHandle_t lock;
        int count;
    } count_button_t;

count_button_t count_p_button;

typedef struct {
    int lastButtonState;
    int buttonState;
    clock_t lastDebounceTime;
    int buttonCode; // the SDL_SCANCODE of the button to check
}debounce_t;

void xGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
        xSemaphoreGive(buttons.lock);
    }
}

int manageGameLogicTasks(tasks_and_game_objects_t *tasks_and_game_objects) {
    // WARNING, this needs to be called from a function wich has the game_info->lock already taken

    /* pause/start all the game calculation tasks if the game is paused/restarted*/

    if (tasks_and_game_objects->game_info->game_state == GAME_PAUSED  || tasks_and_game_objects->game_info->game_state == GAME_PRE_START) {
        for (int i = 0; i < tasks_and_game_objects->game_task_handlers->length; i++) {
            vTaskSuspend(tasks_and_game_objects->game_task_handlers->tasks[i]);
        }
        // suspend both mothership tasks even though only one should be running
        vTaskSuspend(tasks_and_game_objects->playmode_task_handlers->tasks[PLAYMODE_SINGEPLAYER]);

    } else if (tasks_and_game_objects->game_info->game_state == GAME_RUNNING) {
        for (int i = 0; i < tasks_and_game_objects->game_task_handlers->length; i++) {
            vTaskResume(tasks_and_game_objects->game_task_handlers->tasks[i]);
        }
        // resume the right mothership task
        // WARNING game_info->lock needs to be taken
        if (tasks_and_game_objects->game_info->playmode == PLAYMODE_AI_PLAYER) {
            vTaskResume(tasks_and_game_objects->playmode_task_handlers->tasks[PLAYMODE_AI_PLAYER]);
        } else {
            vTaskResume(tasks_and_game_objects->playmode_task_handlers->tasks[PLAYMODE_SINGEPLAYER]);
        }
    }
}

int debounceInitValues(debounce_t *debounce_values, int SDL_SCANCODE) {
    debounce_values->buttonCode = SDL_SCANCODE;
    debounce_values->buttonState = 0;
    debounce_values->lastDebounceTime = clock();
    debounce_values->lastButtonState = 0;
    return EXIT_SUCCESS;
}

int debounceButton(debounce_t *debounce_values) {
    /*!
    @brief checks if a button is pressed with debouncing it.
    @returns 1 if buttons is pressed
    @returns 0 if buttons is not pressed
    @returns NULL if the state is undefined
    */

    int button_pressed = NULL;
    if (buttons.buttons[debounce_values->buttonCode] != debounce_values->lastButtonState) {
        debounce_values->lastDebounceTime = clock();
    }
    if ((clock() - debounce_values->lastDebounceTime) > DEBOUNCE_DELAY) {
        if (buttons.buttons[debounce_values->buttonCode] != debounce_values->buttonState) {
            debounce_values->buttonState = buttons.buttons[debounce_values->buttonCode];

            if (debounce_values->buttonState) {
                button_pressed = 1;
            } else {
                button_pressed = 0;
            }

        }
    } 
    debounce_values->lastButtonState = buttons.buttons[debounce_values->buttonCode];
    return button_pressed;
}

void vManageButtonInputTask(tasks_and_game_objects_t *tasks_and_game_objects){
    game_objects_t *my_gameobjects = tasks_and_game_objects->game_objects;

    // init debounce values
    debounce_t *I_Debounce = pvPortMalloc(sizeof(debounce_t));
    debounceInitValues(I_Debounce, KEYCODE(I));
    debounce_t *S_Debounce = pvPortMalloc(sizeof(debounce_t));
    debounceInitValues(S_Debounce, KEYCODE(S));
    debounce_t *L_Debounce = pvPortMalloc(sizeof(debounce_t));
    debounceInitValues(L_Debounce, KEYCODE(L));
    debounce_t *P_Debounce = pvPortMalloc(sizeof(debounce_t));
    debounceInitValues(P_Debounce, KEYCODE(P));

    while (1) {
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK | FETCH_EVENT_NO_GL_CHECK); // Query events backend for new events, ie. button presses
        xGetButtonInput(); // Update global input

        if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
            // manage buttons to pause / exit the game

            if (xSemaphoreTake(tasks_and_game_objects->game_info->lock, 0) == pdTRUE) {

                // manage buttons only available in the pre game menu
                if (tasks_and_game_objects->game_info->game_state == GAME_PRE_START) {
                    if (buttons.buttons[KEYCODE(Q)]) { // Equiv to SDL_SCANCODE_Q
                        exit(EXIT_SUCCESS);
                    }                    
                    if (buttons.buttons[KEYCODE(S)]) { // use S to start the game
                        tasks_and_game_objects->game_info->game_state = GAME_RUNNING;
                    }
                    if (buttons.buttons[KEYCODE(C)]) { // use C to get to the cheat menu
                        tasks_and_game_objects->game_info->game_state = GAME_CHEAT_MENU;
                    }
                    // toggle game mode between single and AI. Also toggle the running task
                    if (debounceButton(P_Debounce) == 1) {                        
                        if (tasks_and_game_objects->game_info->playmode == PLAYMODE_SINGEPLAYER) {
                            tasks_and_game_objects->game_info->playmode = PLAYMODE_AI_PLAYER;
                            /*vTaskSuspend(tasks_and_game_objects->playmode_task_handlers->tasks[PLAYMODE_SINGEPLAYER]);
                            vTaskResume(tasks_and_game_objects->playmode_task_handlers->tasks[PLAYMODE_AI_PLAYER]);
                            if (xSemaphoreTake(tasks_and_game_objects->game_objects->mothership->lock, 0) == pdTRUE) {
                                tasks_and_game_objects->game_objects->mothership->active == OBJ_ACTIVE;
                                xSemaphoreGive(tasks_and_game_objects->game_objects->mothership->lock);
                            }*/
                        } else {
                            tasks_and_game_objects->game_info->playmode = PLAYMODE_SINGEPLAYER;
                            /*vTaskSuspend(tasks_and_game_objects->playmode_task_handlers->tasks[PLAYMODE_AI_PLAYER]);
                            vTaskResume(tasks_and_game_objects->playmode_task_handlers->tasks[PLAYMODE_SINGEPLAYER]);*/
                        }
                    }

                }

                // manage buttons only available in Cheat Menu
                if (tasks_and_game_objects->game_info->game_state == GAME_CHEAT_MENU) {
                    if (debounceButton(I_Debounce) == 1) { // use I to activate infite lives
                        if (xSemaphoreTake(tasks_and_game_objects->game_objects->score->lock, 0) == pdTRUE) {
                            tasks_and_game_objects->game_objects->score->infitive_lifes = !tasks_and_game_objects->game_objects->score->infitive_lifes;
                            xSemaphoreGive(tasks_and_game_objects->game_objects->score->lock);
                        }
                    }
                    if (debounceButton(S_Debounce)) {  // set the start score
                        if (buttons.buttons[KEYCODE(UP)]) {
                            if (xSemaphoreTake(tasks_and_game_objects->game_objects->score->lock, 0) == pdTRUE) {
                                tasks_and_game_objects->game_objects->score->current_score += 500;
                                xSemaphoreGive(tasks_and_game_objects->game_objects->score->lock);
                            } 
                        }  
                        if (buttons.buttons[KEYCODE(DOWN)]) {
                            if (xSemaphoreTake(tasks_and_game_objects->game_objects->score->lock, 0) == pdTRUE) {
                                tasks_and_game_objects->game_objects->score->current_score -= 500;
                                xSemaphoreGive(tasks_and_game_objects->game_objects->score->lock);
                            } 
                        }                       
                    }

                    if (debounceButton(L_Debounce)) {  // set the start level
                        if (buttons.buttons[KEYCODE(UP)]) {
                            if (xSemaphoreTake(tasks_and_game_objects->game_objects->score->lock, 0) == pdTRUE) {
                                tasks_and_game_objects->game_objects->score->level += 1;
                                xSemaphoreGive(tasks_and_game_objects->game_objects->score->lock);
                            } 
                        }  
                        if (buttons.buttons[KEYCODE(DOWN)]) {
                            if (xSemaphoreTake(tasks_and_game_objects->game_objects->score->lock, 0) == pdTRUE) {
                                tasks_and_game_objects->game_objects->score->level -= 1;
                                xSemaphoreGive(tasks_and_game_objects->game_objects->score->lock);
                            } 
                        }                       
                    }

                    if (buttons.buttons[KEYCODE(M)]) {  // Quit to main menu        
                        tasks_and_game_objects->game_info->game_state = GAME_PRE_START; // GAME_PRE_START;
                    }
                }


                if (tasks_and_game_objects->game_info->game_state == GAME_RUNNING) {
                    if (buttons.buttons[KEYCODE(P)]) { // use p to pause the game                
                        tasks_and_game_objects->game_info->game_state = GAME_PAUSED;
                    }
                    // check if the player lost and if so store the highscore
                    if (xSemaphoreTake(tasks_and_game_objects->game_objects->score->lock, 0) == pdTRUE) {
                        if (tasks_and_game_objects->game_objects->score->lifes_left < 0) {
                            tasks_and_game_objects->game_info->game_state = GAME_ENDED;
                            if (tasks_and_game_objects->game_objects->score > tasks_and_game_objects->game_info->highscore) {
                                tasks_and_game_objects->game_info->highscore = tasks_and_game_objects->game_objects->score->current_score;
                            }
                        }
                        xSemaphoreGive(tasks_and_game_objects->game_objects->score->lock);
                    }
                }
                
                // manage buttons if game is ended
                if (tasks_and_game_objects->game_info->game_state == GAME_ENDED) {
                    if (buttons.buttons[KEYCODE(M)]) {  // Quit to main menu
                        game_objects_init(tasks_and_game_objects->game_objects);  // reset the game object values to the inital value            
                        tasks_and_game_objects->game_info->game_state = GAME_PRE_START; // GAME_PRE_START;
                    }
                }


                // manage buttons only available in the pause menu
                if (tasks_and_game_objects->game_info->game_state == GAME_PAUSED) {
                    if (buttons.buttons[KEYCODE(C)]) { // if the game is paused continue the game                    
                        tasks_and_game_objects->game_info->game_state = GAME_RUNNING;
                    }
                    if (buttons.buttons[KEYCODE(R)]) {  // restart the game
                        game_objects_init(tasks_and_game_objects->game_objects);  // reset the game object values to the inital value
                        tasks_and_game_objects->game_info->game_state = GAME_RUNNING; // GAME_PRE_START;
                    }
                    if (buttons.buttons[KEYCODE(M)]) {  // Quit to main menu
                        game_objects_init(tasks_and_game_objects->game_objects);  // reset the game object values to the inital value            
                        tasks_and_game_objects->game_info->game_state = GAME_PRE_START; // GAME_PRE_START;
                    }
                }

                manageGameLogicTasks(tasks_and_game_objects);  // WARNING, this needs to be called from a function wich has the game_info->lock already taken

                
                // manage buttons only available if the game is running
                if (tasks_and_game_objects->game_info->game_state == GAME_RUNNING) {
                    // manage in game buttons
                    if (buttons.buttons[KEYCODE(LEFT)]) {
                        SpaceShipMoveLeft(my_gameobjects->my_spaceship);
                    }
                    if (buttons.buttons[KEYCODE(RIGHT)]) {
                        SpaceShipMoveRight(my_gameobjects->my_spaceship);
                    }
                    if (buttons.buttons[KEYCODE(SPACE)]) { // shot bullet
                        BulletShoot(my_gameobjects->my_spaceship, my_gameobjects->my_bullet);
                    }
                }

                xSemaphoreGive(tasks_and_game_objects->game_info->lock);
            }
            xSemaphoreGive(buttons.lock);
        }
        vTaskDelay((TickType_t) (SCREEN_FREQUENCY));
    }
}

int MangageButtonInit(tasks_and_game_objects_t *tasks_and_game_objects) {
    // init the Manage Button Task with a higher priority then all the other task. This task is responsible for managing all the other tasks
    if (xTaskCreate(vManageButtonInputTask, "ManageButtonInputTask", mainGENERIC_STACK_SIZE * 2, tasks_and_game_objects,
                    mainGENERIC_PRIORITY + 2, &ManageButtonTask) != pdPASS) {
        printf("Failed to create Tast MangaeButtonInit");
        goto err_demotask;
    }

    buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism

    return EXIT_SUCCESS;

    err_demotask:
        //vSemaphoreDelete(buttons.lock);
        return EXIT_FAILURE;
}