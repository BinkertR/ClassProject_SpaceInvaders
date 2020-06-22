#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Draw.h"

#include "my_structs.h"
#include "spaceship.h"
#include "manage_button_input.h"

int SpaceShipDraw(spaceship_t *my_spaceship) {
    // draw the spaceship according to its current position
    coord_t gun_draw_position, ship_draw_positoin;

    if (xSemaphoreTake((*my_spaceship).lock, 0) == pdTRUE) {

        gun_draw_position.y = my_spaceship->position.y - GUN_HEIGHT;
        gun_draw_position.x = my_spaceship->position.x - GUN_WIDHT / 2 ;

        ship_draw_positoin.y = my_spaceship->position.y;
        ship_draw_positoin.x = my_spaceship->position.x - SHIP_WIDTH / 2;

        xSemaphoreGive((*my_spaceship).lock);

        tumDrawFilledBox(gun_draw_position.x, gun_draw_position.y, GUN_WIDHT, GUN_HEIGHT, Green);
        tumDrawFilledBox(ship_draw_positoin.x, ship_draw_positoin.y, SHIP_WIDTH, SHIP_HEIGHT, Green);

        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

int SpaceShipMoveLeft(spaceship_t *my_spaceship) {
    if (xSemaphoreTake((*my_spaceship).lock, 0) == pdTRUE) {
        my_spaceship->position.x -= SHIP_SPEED;
        if (my_spaceship->position.x < SHIP_X_MIN) {
            my_spaceship->position.x = SHIP_X_MIN;
        }
        xSemaphoreGive(my_spaceship->lock);
        return 0;
    }
    return -1;
}

int SpaceShipMoveRight(spaceship_t *my_spaceship) {
    if (xSemaphoreTake((*my_spaceship).lock, 0) == pdTRUE) {
        my_spaceship->position.x += SHIP_SPEED;
        if (my_spaceship->position.x > SHIP_X_MAX) {
            my_spaceship->position.x = SHIP_X_MAX;
        }
        xSemaphoreGive(my_spaceship->lock);
        return 0;
    }
    return -1;
}
