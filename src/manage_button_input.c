#include <stddef.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Event.h"

#include "spaceship.h"


#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

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

void vManageButtonInputTask(spaceship_t *my_spaceship){


    while (1) {
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK | FETCH_EVENT_NO_GL_CHECK); // Query events backend for new events, ie. button presses
        xGetButtonInput(); // Update global input

        if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
            if (buttons.buttons[KEYCODE(Q)]) { // Equiv to SDL_SCANCODE_Q
                exit(EXIT_SUCCESS);
            }

            if (buttons.buttons[KEYCODE(LEFT)]) {
                SpaceShipMoveLeft(my_spaceship);
            }
            if (buttons.buttons[KEYCODE(RIGHT)]) {
                SpaceShipMoveRight(my_spaceship);
            }

            xSemaphoreGive(buttons.lock);
        }
        vTaskDelay((TickType_t) (1000/30));
    }
}

int MangageButtonInit(spaceship_t *my_spaceship) {
    if (xTaskCreate(vManageButtonInputTask, "ManageButtonInputTask", mainGENERIC_STACK_SIZE * 2, my_spaceship,
                    mainGENERIC_PRIORITY, &ManageButtonTask) != pdPASS) {
        printf("Failed to create Tast MangaeButtonInit");
        goto err_demotask;
    }

    buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism

    return EXIT_SUCCESS;

    err_demotask:
        //vSemaphoreDelete(buttons.lock);
        return EXIT_FAILURE;
}