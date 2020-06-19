#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Draw.h"

#include "spaceship.h"
#include "manage_button_input.h"

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

#define SHIP_HEIGHT     10
#define SHIP_WIDTH      50
#define GUN_HEIGHT      10
#define GUN_WIDHT       10
#define PLAYER_LIFES    2
#define SHIP_SPEED      5

#define SHIP_X_MAX      SCREEN_WIDTH - 10- SHIP_WIDTH / 2
#define SHIP_X_MIN      10
#define SHIP_Y_CO       SCREEN_HEIGHT - 10 - SHIP_HEIGHT


spaceship_t *SpaceShipInit() {
    spaceship_t *my_spaceship = pvPortMalloc(sizeof(spaceship_t));
    int x = 0;
    x = SCREEN_WIDTH / 2 - SHIP_WIDTH / 2;
    //my_spaceship->x_position = pvPortMalloc(sizeof(int));
    my_spaceship->x_position = x;
    my_spaceship->lifes = PLAYER_LIFES;
    my_spaceship->lock = xSemaphoreCreateMutex(); // Locking mechanism

    return my_spaceship;
}

int SpaceShipDraw(spaceship_t *my_spaceship) {
    // draw the spaceship according to its current position
    coord_t gun_draw_position, ship_draw_positoin;

    if (xSemaphoreTake((*my_spaceship).lock, 0) == pdTRUE) {

        gun_draw_position.y = SHIP_Y_CO - GUN_HEIGHT;
        gun_draw_position.x = (*my_spaceship).x_position - GUN_WIDHT / 2 ;

        ship_draw_positoin.y = SHIP_Y_CO;
        ship_draw_positoin.x = (*my_spaceship).x_position - SHIP_WIDTH / 2;

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
        (*my_spaceship).x_position -= SHIP_SPEED;
        if ((*my_spaceship).x_position < SHIP_X_MIN) {
            (*my_spaceship).x_position = SHIP_X_MIN;
        }
        xSemaphoreGive((*my_spaceship).lock);
        return 0;
    }
    return -1;
}

int SpaceShipMoveRight(spaceship_t *my_spaceship) {
    if (xSemaphoreTake((*my_spaceship).lock, 0) == pdTRUE) {
        (*my_spaceship).x_position += SHIP_SPEED;
        if ((*my_spaceship).x_position > SHIP_X_MAX) {
            (*my_spaceship).x_position = SHIP_X_MAX;
        }
        xSemaphoreGive((*my_spaceship).lock);
        return 0;
    }
    return -1;
}
