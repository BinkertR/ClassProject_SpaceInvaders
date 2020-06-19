#include "FreeRTOS.h"
#include "semphr.h"
#include "TUM_Draw.h"

#ifndef TEST_BULLET_H

#define TEST_BULLET_H  

typedef struct{
    coord_t position;
    int active;
    SemaphoreHandle_t lock;
}bullet_t;

bullet_t *BulletInit();

int BulletShoot(spaceship_t *my_spaceship, bullet_t *my_bullet);

int BulletDraw(bullet_t *my_bullet);

void vCalcBulletTask(game_objects_t *my_gameobjects);


#endif /*TEST_BULLET_H*/