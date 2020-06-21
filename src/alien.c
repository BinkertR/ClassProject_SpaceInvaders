#include <stdio.h>

#include "TUM_Draw.h"

#include "my_structs.h"

#define ALIEN_EASY_IMG "../img/alien_green.png"
#define ALIEN_MIDDLE_IMG "../img/alien_yellow.png"
#define ALIEN_HARD_IMG "../img/alien_pink.png"

TaskHandle_t AlienCalcSingleTask = NULL;
TaskHandle_t AlienCalcMatrixTask = NULL;

image_handle_t alien_low_img_handle = NULL;
image_handle_t alien_medium_img_handle = NULL;
image_handle_t alien_high_img_handle = NULL;

image_handle_t AlienLoadImg(int alien_score) {  // TODO 
    image_handle_t alien_img;  //TODO store this so the same img doesn't need to be loaded multiple times
    int img_width = 0;
    float scale_factor = 1.0;
    if (alien_score == ALIEN_EASY) {
        if (alien_low_img_handle == NULL) {
            alien_low_img_handle = tumDrawLoadImage(ALIEN_EASY_IMG);  
            img_width = tumDrawGetLoadedImageWidth(alien_low_img_handle);
            scale_factor = ALIEN_WIDTH * 1.0 / img_width;
            tumDrawSetLoadedImageScale(alien_low_img_handle, scale_factor);  
        }
        alien_img = alien_low_img_handle;
    } else if (alien_score == ALIEN_MIDDLE) {
        if (alien_medium_img_handle == NULL) {
            alien_medium_img_handle = tumDrawLoadImage(ALIEN_MIDDLE_IMG);
            img_width = tumDrawGetLoadedImageWidth(alien_medium_img_handle);
            scale_factor = ALIEN_WIDTH * 1.0 / img_width;
            tumDrawSetLoadedImageScale(alien_medium_img_handle, scale_factor);  
        }
        alien_img = alien_medium_img_handle;
    } else {
        if (alien_high_img_handle == NULL) {
            alien_high_img_handle = tumDrawLoadImage(ALIEN_HARD_IMG);
            img_width = tumDrawGetLoadedImageWidth(alien_high_img_handle);
            scale_factor = ALIEN_WIDTH * 1.0 / img_width;
            tumDrawSetLoadedImageScale(alien_high_img_handle, scale_factor);  
        }
        alien_img = alien_high_img_handle;
    }
    if (alien_img == NULL) {
        printf("Failed to load alien img");
        return NULL;
    }
    
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
        current_alien = alien_column_start + i;
        AlienDrawSingle(*current_alien);
    }
    return 0;
}

int AlienDrawMatrix(alien_t ***alien_matrix_start) {
    alien_t ***current_row = alien_matrix_start;

    for (int i = 0; i < ALIENS_PER_ROW; i++) {
        current_row = alien_matrix_start + i;
        AlienDrawColumn(*current_row);
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

int AlienCheckBullet(alien_t *my_alien, bullet_t *my_bullet) {
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
}

void vAlienCalcMatrixTask(game_objects_t *my_gameobjects){
    alien_t *my_alien = pvPortMalloc(sizeof(alien_t));
    alien_t ***current_row = pvPortMalloc(sizeof(alien_t) * ALIENS_PER_ROW);
    alien_t **current_column = pvPortMalloc(sizeof(alien_t) * ALIENS_PER_COLUMN);
    bullet_t *my_bullet = pvPortMalloc(sizeof(bullet_t));
    
    my_bullet = my_gameobjects->my_bullet;
    int matrix_size_x = ALIENS_PER_ROW, matrix_size_y = ALIENS_PER_COLUMN;

    while (1) {
        for (int i = 0; i < matrix_size_x; i++) {  // iterate through rows
            current_row = my_gameobjects->alien_matrix + i;
            for (int k = 0; k < matrix_size_y; k++) { // iterate through columns
                current_column = *current_row + k;
                AlienCheckBullet(*current_column, my_bullet);
            }
        }
                
        vTaskDelay((TickType_t) (SCREEN_FREQUENCY));
    }
}

int AlienInitCalcMatrixTask(game_objects_t *my_game_objects) {
    if (xTaskCreate(vAlienCalcMatrixTask, "AlienCalcMatrixTask", mainGENERIC_STACK_SIZE * 2, my_game_objects,
                    mainGENERIC_PRIORITY, &AlienCalcMatrixTask) != pdPASS) {
        printf("Failed to create Task AlienCalcMatrixTask");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
