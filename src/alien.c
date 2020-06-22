#include <stdio.h>
#include <stdlib.h>

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
        if (my_alien->active == OBJ_ACTIVE) {
            img = my_alien->img_h;
            tumDrawLoadedImage(img, alien_draw_position.x, alien_draw_position.y);
        }
        xSemaphoreGive((*my_alien).lock);   
    }    
    return 0;
}

int AlienDrawMatrix(game_objects_t *my_gameobjects) {  
    alien_t *my_alien = pvPortMalloc(sizeof(alien_t));
    
    int matrix_size_x = ALIENS_PER_ROW, matrix_size_y = ALIENS_PER_COLUMN;
    
    for (int i = 0; i < matrix_size_x; i++) {  // iterate through rows
        //current_row = &(my_gameobjects->alien_matrix[i]);
        for (int k = 0; k < matrix_size_y; k++) { // iterate through columns
            my_alien = my_gameobjects->alien_matrix->first_column[i]->first_alien[k];
            //current_column = *current_row->first_column + k;
            AlienDrawSingle(my_alien);
        }
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
    // checks threadsafe if the alien got hit by the bullet
    // returns 1, if the alien got hit, returns 0 else

    // just call this function from where the alien lock is already taken

    int got_hit = 0;
    if (my_alien->active == OBJ_ACTIVE) {
        if (xSemaphoreTake(my_bullet->lock, 0) == pdTRUE) {
            if (my_bullet->active == OBJ_ACTIVE) {
                //if the bullet hits the alien
                if (check_hit(my_alien, my_bullet) == 1) {
                    my_alien->active = OBJ_PASSIVE;
                    my_bullet->active = OBJ_PASSIVE;
                    got_hit = 1;
                } 
            }                    
            xSemaphoreGive(my_bullet->lock);
        }                
    }
    return got_hit;
}

int AlienIterateMatrix(game_objects_t *my_gameobjects, float *current_alien_speed, float step_in_x, float step_in_y) {
    // check for each active alien, if it got hit by the bullet
    // if it got hit set its status to passive and if applicable the status of the hole column
    // if an alien got hit, also adapt the current_alien_speed
    // move each alien according to the previously calculated steps in x and y direction

    // define some vars for better readability
    alien_column_t *current_column = pvPortMalloc(sizeof(alien_column_t));  
    alien_t *current_alien = pvPortMalloc(sizeof(alien_t));
    alien_matrix_t *alien_matrix = pvPortMalloc(sizeof(alien_matrix_t));
    alien_column_t *leftest_active_column = pvPortMalloc(sizeof(alien_column_t));
    alien_column_t *rightest_active_column = pvPortMalloc(sizeof(alien_column_t));

    alien_matrix = my_gameobjects->alien_matrix;

    if (xSemaphoreTake(alien_matrix->lock, 0) == pdTRUE) {

        for (int i = alien_matrix->leftest_active_column; i <= alien_matrix->rightest_active_column; i++) {  // iterate through rows
            current_column = my_gameobjects->alien_matrix->first_column[i];
            if (xSemaphoreTake(current_column->lock, 0) == pdTRUE) {
                if (current_column->active == OBJ_ACTIVE) {
                    current_column->lowest_active_alien = -1;  // set the value to no active alien. This will be updated each time an active alien in this column is found
                    for (int k = 0; k < ALIENS_PER_COLUMN; k++) { // iterate through columns
                        current_alien = my_gameobjects->alien_matrix->first_column[i]->first_alien[k];

                        if (xSemaphoreTake(current_alien->lock, 0) == pdTRUE) {
                            
                            // if the alien is active, calculate, if it gets hit by the bullet
                            if (current_alien->active == OBJ_ACTIVE) {
                                // if alien got hit: update speed and score, else: update position and lowest_active_alien
                                if (AlienCheckBullet(current_alien, my_gameobjects->my_bullet) == 1) {  // for this the alien_lock needs to be taken
                                    // if the alien got hit
                                    //increase the speed
                                    *(current_alien_speed) += ALIEN_ACCELERATION;
                                    // increase the score
                                    if (xSemaphoreTake(my_gameobjects->score->lock, 0) == pdTRUE) {
                                        my_gameobjects->score->current_score += current_alien->alien_score;
                                        xSemaphoreGive(my_gameobjects->score->lock);
                                    }
                                } else {
                                    // update the lowest active alien value of the column to this alien
                                    current_column->lowest_active_alien = k;
                                
                                    // move alien according to the previously calculated steps
                                    current_alien->position.x += step_in_x;
                                    current_alien->position.y += step_in_y;
                                }
                            } 

                            xSemaphoreGive(current_alien->lock);
                        }                  
                                    
                    }
                    if (current_column->lowest_active_alien == -1) {
                        // if all the aliens in this column is not active anymore set the column to passive
                        current_column->active = OBJ_PASSIVE;
                        my_gameobjects->alien_matrix->active_columns[i] = OBJ_PASSIVE;
                    }   
                }

                xSemaphoreGive(current_column->lock);
            }
 
        } 
        xSemaphoreGive(alien_matrix->lock);
    }
    
    return 0; 
}

int GetRandInt() {
    // get a random number between 0 and 1000
    int random_int;

    random_int = (rand() % (RAND_SHOOT_CHANCE - 0 + 1)) + 0;

    return random_int;
}

int AlienShootBullet(game_objects_t *gameobjects) {
    // iterate through all active columns and give the lowest alien a chance to shoot a bullet
    alien_column_t *current_column; // = pvPortMalloc(sizeof(alien_column_t));
    bullet_t *current_bullet; // = pvPortMalloc(sizeof(bullet_t));

    int r = 0, lowest_active_alien = 0;

    for (int i = 0; i < ALIENS_PER_ROW; i++) {
        current_column = gameobjects->alien_matrix->first_column[i];
        if (xSemaphoreTake(current_column->lock, 0) == pdTRUE) {
            if (current_column->active == OBJ_ACTIVE) {
                r = GetRandInt();
                if (r <= 1) {
                    lowest_active_alien = current_column->lowest_active_alien;
                    for ( int k = 0; k < MAX_ACTIVE_ALIEN_BULLETS; k++) {  // get the first free alien bullet slot
                        current_bullet = gameobjects->alien_bullets[k];
                        if (xSemaphoreTake(current_bullet->lock, 0) == pdTRUE) {
                            if (current_bullet->active == OBJ_PASSIVE) {
                                current_bullet->position.x = current_column->first_alien[lowest_active_alien]->position.x;
                                current_bullet->position.y = current_column->first_alien[lowest_active_alien]->position.y;
                                current_bullet->active = OBJ_ACTIVE;
                                xSemaphoreGive(current_bullet->lock);
                                break;  // if one free bullet slot was found and the bullet was shot. End the search for the free bullet slot
                            }
                            xSemaphoreGive(current_bullet->lock);
                        }
                    }
                }
            }
            xSemaphoreGive(current_column->lock);
        }  
    }
    return 0;
    
}

void vAlienCalcMatrixTask(game_objects_t *my_gameobjects){
    alien_t *my_alien = pvPortMalloc(sizeof(alien_t));
    alien_matrix_t *alien_matrix = pvPortMalloc(sizeof(alien_matrix_t));
    alien_column_t *current_column = pvPortMalloc(sizeof(alien_column_t));
    alien_column_t *leftest_active_column = pvPortMalloc(sizeof(alien_column_t));
    alien_column_t *rightest_active_column = pvPortMalloc(sizeof(alien_column_t));
    bullet_t *my_bullet = pvPortMalloc(sizeof(bullet_t));

    int current_min_x = NULL, current_max_x = NULL, leftest_active_column_int = 0, rightest_active_column_int = ALIENS_PER_ROW - 1;  // how fare the aliens should step into each direction
    float step_in_x = ALIEN_X_SPEED, step_in_y = ALIEN_Y_SPEED;
    float current_alien_speed = ALIEN_X_SPEED;

    my_bullet = my_gameobjects->my_bullet;
    alien_matrix = my_gameobjects->alien_matrix;
    leftest_active_column = my_gameobjects->alien_matrix->first_column[leftest_active_column_int];
    rightest_active_column = my_gameobjects->alien_matrix->first_column[rightest_active_column_int];

    while (1) {
        //set the moving steps, so that they can be applied to each alien during the bullet iteration
        // get the x value of the most left and the most right alien in the hole matrix to see if these aliens would be out of the screen
        current_min_x = leftest_active_column->first_alien[leftest_active_column->lowest_active_alien]->position.x;
        current_max_x = rightest_active_column->first_alien[rightest_active_column->lowest_active_alien]->position.x;
        if (current_min_x - ALIEN_WIDTH - current_alien_speed / 2 < ALIEN_MIN_X) {  // if the aliens are on the left border change direction
            step_in_x = + current_alien_speed;
            step_in_y = 0;
        } else if (current_max_x + ALIEN_WIDTH + current_alien_speed / 2 > ALIEN_MAX_X) {  // if the aliens are on the left border change direction and move aliens one down
            step_in_x = - current_alien_speed;
            step_in_y = ALIEN_Y_SPEED;
        } else {  // continue moving aliens in the same direction, but stop going down
            step_in_y = 0;
            if (step_in_x < 0) {
                step_in_x = - current_alien_speed;
            } else {
                step_in_x = + current_alien_speed;
            }
        }

        AlienIterateMatrix(my_gameobjects, &current_alien_speed, step_in_x, step_in_y);

        //update leftest and rightes active column
        leftest_active_column_int = ALIENS_PER_ROW - 1; 
        rightest_active_column_int = 0;
        for (int i = 0; i < ALIENS_PER_ROW; i++) {
            current_column = my_gameobjects->alien_matrix->first_column[i];
            if (current_column->active == OBJ_ACTIVE) {
                if (i < leftest_active_column_int) {
                    leftest_active_column_int = i;
                }
                if (i > rightest_active_column_int) {
                    rightest_active_column_int = i;
                }
            }
        }
        if (leftest_active_column_int == ALIENS_PER_ROW - 1 && rightest_active_column_int == 0) {
            // player killed all aliens in this stage
            // reload aliens for next level
            printf("You won this level");
            if (xSemaphoreTake(my_gameobjects->lock, 0) == pdTRUE) {
                my_gameobjects->alien_matrix = AlienInitMatrix();
                xSemaphoreGive(my_gameobjects->lock);
            }
            if (xSemaphoreTake(my_gameobjects->score->lock, 0) == pdTRUE) {
                my_gameobjects->score->level += 1;
                current_alien_speed = ALIEN_X_SPEED * my_gameobjects->score->level * LEVEL_SPEED_INCREASE_FAKTOR;
                xSemaphoreGive(my_gameobjects->score->lock);
            }
            alien_matrix = my_gameobjects->alien_matrix;
            leftest_active_column = my_gameobjects->alien_matrix->first_column[leftest_active_column_int];
            rightest_active_column = my_gameobjects->alien_matrix->first_column[rightest_active_column_int];
        } else {
            my_gameobjects->alien_matrix->leftest_active_column = leftest_active_column_int;
            my_gameobjects->alien_matrix->rightest_active_column = rightest_active_column_int;
            leftest_active_column = my_gameobjects->alien_matrix->first_column[leftest_active_column_int];
            rightest_active_column = my_gameobjects->alien_matrix->first_column[rightest_active_column_int];

            AlienShootBullet(my_gameobjects);
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
