#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Draw.h"
#include "TUM_Event.h"

#include "my_structs.h"

#define ALIEN_BULLET_COLOR Red;

TaskHandle_t ManageBulletTask = NULL;

int BulletAlienDraw(game_objects_t *gameobjects) {
    bullet_t *current_alien_bullet;
    coord_t bullet_draw_position;

    // draw alien bullets
    for (int i = 0; i < MAX_ACTIVE_ALIEN_BULLETS; i++) {
        current_alien_bullet = gameobjects->alien_bullets[i];
        if (xSemaphoreTake(current_alien_bullet->lock, 0) == pdTRUE) {
            if (current_alien_bullet->active == OBJ_ACTIVE) {
                bullet_draw_position.x = current_alien_bullet->position.x - BULLET_WIDTH / 2;
                bullet_draw_position.y = current_alien_bullet->position.y - BULLET_HEIGHT / 2;

                tumDrawFilledBox(bullet_draw_position.x, bullet_draw_position.y, BULLET_WIDTH, BULLET_HEIGHT, Red);
            }
            xSemaphoreGive(current_alien_bullet->lock);
        }
    }
    return 0;
}

int BulletDraw(bullet_t *my_bullet) {
    // draw the bullet according to its current position (which is update via the manage_button task)
    coord_t bullet_draw_position;

    if (xSemaphoreTake((*my_bullet).lock, 0) == pdTRUE) {
        if (my_bullet->active == OBJ_ACTIVE) {
            
            bullet_draw_position.x = my_bullet->position.x - BULLET_WIDTH / 2;
            bullet_draw_position.y = my_bullet->position.y - BULLET_HEIGHT / 2;

            tumDrawFilledBox(bullet_draw_position.x, bullet_draw_position.y, BULLET_WIDTH, BULLET_HEIGHT, Green);
        }
        
        xSemaphoreGive((*my_bullet).lock);      

        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

int BulletShoot(spaceship_t *my_spaceship, bullet_t *my_bullet) {
    if (xSemaphoreTake((*my_bullet).lock, 0) == pdTRUE) {
        if (my_bullet->active == OBJ_PASSIVE) {
            my_bullet->active = OBJ_ACTIVE;
            if (xSemaphoreTake((*my_spaceship).lock, 0) == pdTRUE) {
                my_bullet->position.x = my_spaceship->position.x;  // shoot the bullet from the center of the x of the spaceship
                my_bullet->position.y = SHIP_Y_CO - SHIP_HEIGHT - GUN_HEIGHT - BULLET_HEIGHT / 2;
                xSemaphoreGive(my_spaceship->lock);
            }
        }
        xSemaphoreGive(my_bullet->lock);
        return 0;
    } else {
        // if bullet is already active dont activate it again
        // -> do nothing
        return 0;
    }
}

int check_ship_hit(spaceship_t *my_spaceship, bullet_t *my_bullet) {
    int hit = 0;
    if (my_spaceship->position.x - SHIP_WIDTH / 2 < my_bullet->position.x + ALIEN_BULLET_SPEED && my_bullet->position.x - BULLET_WIDTH < my_spaceship->position.x + SHIP_WIDTH / 2 ) {
        if (my_spaceship->position.y - (SHIP_HEIGHT + GUN_HEIGHT) / 2 < my_bullet->position.y && my_bullet->position.y < my_spaceship->position.y + (SHIP_HEIGHT + GUN_HEIGHT) / 2 ) {
            hit = 1;
        }
    } 
    return hit;
}

void vCalcBulletsTask(game_objects_t *my_gameobjects){
    bullet_t *current_alien_bullet = pvPortMalloc(sizeof(bullet_t));

    while (1) {
        // first calc spaceship bullet
        if (xSemaphoreTake(my_gameobjects->my_bullet->lock, 0) == pdTRUE) {
            if (my_gameobjects->my_bullet->active == OBJ_ACTIVE) {
                my_gameobjects->my_bullet->position.y -= BULLET_SPEED;
                if (my_gameobjects->my_bullet->position.y < PADDING) {
                    my_gameobjects->my_bullet->active = OBJ_PASSIVE;
                }
            }
            xSemaphoreGive(my_gameobjects->my_bullet->lock);
        }
        // calc alien bullets
        for (int i = 0; i < MAX_ACTIVE_ALIEN_BULLETS; i++) {
            current_alien_bullet = my_gameobjects->alien_bullets[i];
            if (xSemaphoreTake(current_alien_bullet->lock, 0) == pdTRUE) {
                if (current_alien_bullet->active == OBJ_ACTIVE) {
                    // move alien_bullet
                    current_alien_bullet->position.y += ALIEN_BULLET_SPEED;

                    // check if ship got hit by bullet
                    if (xSemaphoreTake(my_gameobjects->my_spaceship->lock, 0) == pdTRUE) {
                        if (check_ship_hit(my_gameobjects->my_spaceship, current_alien_bullet)) {
                            current_alien_bullet->active = OBJ_PASSIVE; 
                            if (xSemaphoreTake(my_gameobjects->score->lock, 0) == pdTRUE) {
                                if (my_gameobjects->score->infitive_lifes == 0) {
                                    my_gameobjects->score->lifes_left -= 1;
                                } else {
                                    printf("Ship got hit but infitive lifes cheat is activated.");
                                }
                                xSemaphoreGive(my_gameobjects->score->lock);
                            }
                        }
                        xSemaphoreGive(my_gameobjects->my_spaceship->lock);
                    }
                    

                    if (current_alien_bullet->position.y > SCREEN_HEIGHT - PADDING) {
                        current_alien_bullet->active = OBJ_PASSIVE;
                    }
                }
                xSemaphoreGive(current_alien_bullet->lock);
            }
        }
        
        vTaskDelay((TickType_t) (SCREEN_FREQUENCY));
    }
}

TaskHandle_t BulletInitCalcTask(game_objects_t *my_game_objects) {
    if (xTaskCreate(vCalcBulletsTask, "CalcBulletTask", mainGENERIC_STACK_SIZE * 2, my_game_objects,
                    mainGENERIC_PRIORITY, &ManageBulletTask) != pdPASS) {
        printf("Failed to create Tast MangaeButtonInit");
        return NULL;
    }
    return ManageBulletTask;
}
