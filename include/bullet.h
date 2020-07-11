#include "FreeRTOS.h"
#include "semphr.h"
#include "TUM_Draw.h"

#ifndef TEST_BULLET_H

#define TEST_BULLET_H  

#include "my_structs.h"

int BulletShoot(spaceship_t *my_spaceship, bullet_t *my_bullet);

int BulletAlienDraw(game_objects_t *gameobjects);

int BulletDraw(bullet_t *my_bullet);

TaskHandle_t BulletInitCalcTask(game_objects_t *my_game_objects);

#endif /*TEST_BULLET_H*/