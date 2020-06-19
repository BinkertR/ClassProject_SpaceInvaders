
#include "spaceship.h"
#include "bullet.h"

#ifndef TEST_GAME_OBJECTS_H
#define TEST_GAME_OBJECTS_H  

typedef struct {
    spaceship_t *my_spaceship;
    bullet_t *my_bullet;
} game_objects_t;

game_objects_t *game_objects_init();

#endif /*TEST_GAME_OBJECTS_H*/
