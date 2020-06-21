#include <stdio.h>

#include "TUM_Draw.h"

#include "my_structs.h"

#define ALIEN_EASY_IMG "../img/alien_green.png"
#define ALIEN_MIDDLE_IMG "../img/alien_yellow.png"
#define ALIEN_HARD_IMG "../img/alien_ping.png"

TaskHandle_t AlienCalcSingleTask = NULL;

image_handle_t AlienLoadImg(int alien_score) {
    image_handle_t alien_img;
    int img_width = 0;
    float scale_factor = 1.0;
    if (alien_score == ALIEN_EASY) {
        alien_img = tumDrawLoadImage(ALIEN_EASY_IMG);
    } else if (alien_score == ALIEN_MIDDLE) {
        alien_img = tumDrawLoadImage(ALIEN_MIDDLE_IMG);
    } else {
        alien_img = tumDrawLoadImage(ALIEN_HARD_IMG);
    }
    if (alien_img == -1) {
        printf("Failed to load alien img");
        return 1;
    }
    img_width = tumDrawGetLoadedImageWidth(alien_img);
    scale_factor = ALIEN_WIDTH * 1.0 / img_width;
    tumDrawSetLoadedImageScale(alien_img, scale_factor);
    return alien_img;
}

int AlienDrawSingle(alien_t *my_alien) {
    image_handle_t img;
    coord_t alien_draw_position;
    alien_draw_position.x = my_alien->position.x - ALIEN_WIDTH / 2;
    alien_draw_position.y = my_alien->position.y - ALIEN_WIDTH / 2;  //TODO Make this to alien Height
    if (xSemaphoreTake((*my_alien).lock, 0) == pdTRUE) {
        if (my_alien->active == BULLET_ACTIVE) {
            img = my_alien->img_h;
            tumDrawLoadedImage(img, alien_draw_position.x, alien_draw_position.y);
        }
        xSemaphoreGive((*my_alien).lock);   
    }    
    return 0;
}

int AlienDrawColumn(alien_t **alien_column_start) {
    alien_t **current_alien = alien_column_start;

    for (int i = 0; i < ALIENS_PER_COLUMN; i++) {
        current_alien = alien_column_start + 1;
        //AlienDrawSingle(current_alien);
    }
    return 0;
}

int check_hit(alien_t *my_alien, bullet_t *my_bullet) {
    int hit = 0;
    if (my_alien->position.x - ALIEN_WIDTH / 2 < my_bullet->position.x + BULLET_SPEED && my_bullet->position.x - BULLET_WIDTH < my_alien->position.x + ALIEN_WIDTH / 2 ) {
        if (my_alien->position.y - ALIEN_WIDTH / 2 < my_bullet->position.y && my_bullet->position.y < my_alien->position.y + ALIEN_WIDTH / 2 ) {
            hit = 1;
        }
    } 
    return hit;
}


void vAlienCalcSingleTask(game_objects_t *my_gameobjects){
    alien_t *my_alien = pvPortMalloc(sizeof(alien_t));
    bullet_t *my_bullet = pvPortMalloc(sizeof(bullet_t));
    my_alien = my_gameobjects->my_alien;
    my_bullet = my_gameobjects->my_bullet;


    while (1) {
        
        if (xSemaphoreTake(my_alien->lock, 0) == pdTRUE) {
            if (my_alien->active == BULLET_ACTIVE) {
                if (xSemaphoreTake(my_bullet->lock, 0) == pdTRUE) {
                    if (my_bullet->active == BULLET_ACTIVE) {
                        //if the bullet hits the alien
                        if (check_hit(my_alien, my_bullet) == 1) {
                            my_alien->active = BULLET_PASSIVE;
                            my_bullet->active = BULLET_PASSIVE;
                        } 
                    }                    
                    xSemaphoreGive(my_bullet->lock);
                }                
            }
            xSemaphoreGive(my_alien->lock);
        }
        
        vTaskDelay((TickType_t) (SCREEN_FREQUENCY));
    }
}

int AlienInitCalcSingleTask(game_objects_t *my_game_objects) {
    if (xTaskCreate(vAlienCalcSingleTask, "AlienCalcSingleTask", mainGENERIC_STACK_SIZE * 2, my_game_objects,
                    mainGENERIC_PRIORITY, &AlienCalcSingleTask) != pdPASS) {
        printf("Failed to create Tast MangaeButtonInit");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

