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

int manageGameState(int game_running, taskhandle_array_t *Tasks) {
    /* pause/start all the game calculation tasks if the game is paused/restarted*/
    if (game_running == OBJ_PASSIVE) {
        for (int i = 0; i < Tasks->length; i++) {
            vTaskSuspend(Tasks->tasks[i]);
        }
    } else {
        for (int i = 0; i < Tasks->length; i++) {
            vTaskResume(Tasks->tasks[i]);
        }
    }
    

}

void vManageButtonInputTask(tasks_and_game_objects_t *tasks_and_game_objects){
    game_objects_t *my_gameobjects = tasks_and_game_objects->game_objects;
    taskhandle_array_t  *game_task_handle_array = tasks_and_game_objects->game_task_handlers;
    int game_running = OBJ_ACTIVE;

    while (1) {
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK | FETCH_EVENT_NO_GL_CHECK); // Query events backend for new events, ie. button presses
        xGetButtonInput(); // Update global input

        if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
            // manage buttons to pause / exit the game
            if (buttons.buttons[KEYCODE(Q)]) { // Equiv to SDL_SCANCODE_Q
                exit(EXIT_SUCCESS);
            }
            if (buttons.buttons[KEYCODE(P)]) { // use p to pause the game
                game_running = !game_running;  // TODO debounce this button
                manageGameState(game_running, game_task_handle_array);
            }
            
            if (game_running) {
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