
#include "TUM_Draw.h"

#include "my_structs.h"

bullet_t *BulletInit() {
    bullet_t *my_bullet = pvPortMalloc(sizeof(bullet_t));
    my_bullet->position.x = 0;
    my_bullet->position.y = 0;
    my_bullet->active = BULLET_PASSIVE;
    my_bullet->lock = xSemaphoreCreateMutex(); // Locking mechanism

    return my_bullet;
}


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

game_objects_t *game_objects_init() {
    game_objects_t *game_objects = pvPortMalloc(sizeof(game_objects_t));
    game_objects->my_spaceship = SpaceShipInit();
    game_objects->my_bullet = BulletInit();
    return game_objects;
}