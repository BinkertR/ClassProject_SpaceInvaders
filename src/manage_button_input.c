#include <stddef.h>

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
    } else if (tasks_and_game_objects->game_info->game_state == GAME_RUNNING) {
        for (int i = 0; i < tasks_and_game_objects->game_task_handlers->length; i++) {
            vTaskResume(tasks_and_game_objects->game_task_handlers->tasks[i]);
        }
    }
}

void vManageButtonInputTask(tasks_and_game_objects_t *tasks_and_game_objects){
    game_objects_t *my_gameobjects = tasks_and_game_objects->game_objects;
    taskhandle_array_t  *game_task_handle_array = tasks_and_game_objects->game_task_handlers;

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
                        tasks_and_game_objects->game_info->game_state = GAME_RUNNING;
                    }

                }

                if (tasks_and_game_objects->game_info->game_state == GAME_RUNNING) {
                    if (buttons.buttons[KEYCODE(P)]) { // use p to pause the game                
                        tasks_and_game_objects->game_info->game_state = GAME_PAUSED;
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
                    if (buttons.buttons[KEYCODE(SPACE)]) {
                        // shot bullet
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