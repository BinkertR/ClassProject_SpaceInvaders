#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Draw.h"
#include "TUM_Event.h"

#include "my_structs.h"


int BulletDraw(bullet_t *my_bullet) {
    // draw the bullet according to its current position (which is update via the manage_button task)
    coord_t bullet_draw_position;

    if (xSemaphoreTake((*my_bullet).lock, 0) == pdTRUE) {

        bullet_draw_position.x = my_bullet->position.x - BULLET_WIDTH / 2;
        bullet_draw_position.y = my_bullet->position.y - BULLET_HEIGHT / 2;
        
        xSemaphoreGive((*my_bullet).lock);

        tumDrawFilledBox(bullet_draw_position.x, bullet_draw_position.y, BULLET_WIDTH, BULLET_HEIGHT, Green);

        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

int BulletShoot(spaceship_t *my_spaceship, bullet_t *my_bullet) {
    if (xSemaphoreTake((*my_bullet).lock, 0) == pdTRUE) {
        if (my_bullet->active == BULLET_PASSIVE) {
            my_bullet->active = BULLET_ACTIVE;
            if (xSemaphoreTake((*my_spaceship).lock, 0) == pdTRUE) {
                (*my_bullet).position.x = my_spaceship->x_position;  // shoot the bullet from the center of the x of the spaceship
                xSemaphoreGive(my_spaceship->lock);
            }
        }
        xSemaphoreGive((*my_bullet).lock);
        return 0;
    } else {
        // if bullet is already active dont activate it again
        // -> do nothing
        return 0;
    }
}

void vCalcBulletTask(game_objects_t *my_gameobjects){
    bullet_t *my_bullet = pvPortMalloc(sizeof(bullet_t));
    my_bullet = my_gameobjects->my_bullet;

    while (1) {
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK | FETCH_EVENT_NO_GL_CHECK); // Query events backend for new events, ie. button presses
        xGetButtonInput(); // Update global input

        if (xSemaphoreTake(my_gameobjects->my_bullet->lock, 0) == pdTRUE) {
            if (my_gameobjects->my_bullet->active == BULLET_ACTIVE) {
                // move bullet
            }
        }

        
        vTaskDelay((TickType_t) (1000/30));
    }
}
