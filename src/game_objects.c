#include "game_objects.h"
#include "spaceship.h"
#include "bullet.h"

game_objects_t *game_objects_init() {
    game_objects_t *game_objects = pvPortMalloc(sizeof(game_objects_t));
    game_objects->my_spaceship = SpaceShipInit();
    game_objects->my_bullet = BulletInit();
    return game_objects;
}
