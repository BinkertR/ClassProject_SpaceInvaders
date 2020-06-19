
#include "TUM_Draw.h"

#include "my_structs.h"
#include "alien.h"

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
    my_spaceship->x_position = x;
    my_spaceship->lifes = PLAYER_LIFES;
    my_spaceship->lock = xSemaphoreCreateMutex(); // Locking mechanism

    return my_spaceship;
}

alien_t *AlienEasyInt() {
    alien_t *my_alien = pvPortMalloc(sizeof(alien_t));
    my_alien->position.x = 50;
    my_alien->position.y = 50;
    my_alien->alien_score = ALIEN_EASY;
    my_alien->active = BULLET_ACTIVE;  // init to active because it should be display from the beginning
    my_alien->img_h = AlienGreenLoadImg();
    my_alien->lock = xSemaphoreCreateMutex(); // Locking mechanism
    return my_alien;
}

alien_t *AlienInitRow() {
    alien_t *row[ALIENS_PER_ROW];
    int i = 0;
    for (i; i < ALIENS_PER_ROW; i++) {  //sizeof(row) / sizeof(alien_t)
        row[i] = AlienEasyInt();
    }
    return row;
}

game_objects_t *game_objects_init() {
    game_objects_t *game_objects = pvPortMalloc(sizeof(game_objects_t));
    game_objects->my_spaceship = SpaceShipInit();
    game_objects->my_bullet = BulletInit(); 
    game_objects->my_alien = AlienEasyInt();   

    return game_objects;
}