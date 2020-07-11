#include "TUM_Draw.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "my_structs.h"

TaskHandle_t ManageBunkersTask = NULL;

int BunkersDraw(game_objects_t *gameobjects) {
    bunker_t *current_bunker;

    for (int i = 0; i < NUMBER_OF_BUNKERS; i++) {
        current_bunker = gameobjects->bunkers[i];
        if (xSemaphoreTake(current_bunker->lock, 0) == pdTRUE) {            
            for (int k = 0; k < BUNKER_X_CELLS_NUMBER; k++) {
                // draw the active cells of the upper row
                if (current_bunker->upper_row[k].active == OBJ_ACTIVE) {
                    tumDrawFilledBox(current_bunker->upper_row[k].position.x - BUNKER_CELL_SIZE / 2, current_bunker->upper_row[k].position.y - BUNKER_CELL_SIZE / 2, BUNKER_CELL_SIZE, BUNKER_CELL_SIZE, Green);
                }
                // draw the active cells of the middle row
                if (current_bunker->middle_row[k].active == OBJ_ACTIVE) {
                    tumDrawFilledBox(current_bunker->middle_row[k].position.x - BUNKER_CELL_SIZE / 2, current_bunker->middle_row[k].position.y - BUNKER_CELL_SIZE / 2, BUNKER_CELL_SIZE, BUNKER_CELL_SIZE, Green);
                }
                // draw the active cells of the lower row
                if (current_bunker->lower_row[k].active == OBJ_ACTIVE) {
                    tumDrawFilledBox(current_bunker->lower_row[k].position.x - BUNKER_CELL_SIZE / 2, current_bunker->lower_row[k].position.y - BUNKER_CELL_SIZE / 2, BUNKER_CELL_SIZE, BUNKER_CELL_SIZE, Green);
                }

            }
            xSemaphoreGive(current_bunker->lock);
        }
    }
    return 0;
}

int check_bunkercell_hit(bunker_cell_t *bunker_cell, bullet_t *my_bullet) {
    int hit = 0;
    if (bunker_cell->position.x - BUNKER_CELL_SIZE / 2 < my_bullet->position.x + ALIEN_BULLET_SPEED && my_bullet->position.x - BULLET_WIDTH < bunker_cell->position.x + BUNKER_CELL_SIZE / 2 ) {
        if (bunker_cell->position.y - BUNKER_CELL_SIZE / 2 < my_bullet->position.y && my_bullet->position.y < bunker_cell->position.y + BUNKER_CELL_SIZE / 2 ) {
            hit = 1;
        }
    } 
    return hit;
}

int BunkerCheckHit(game_objects_t *game_objects) {
    /*check for all bunker cells, if they got hit by a bullet from the player or the alien.
    
    if it got hit, set the cell and the bullet to passive.
    */
   bullet_t *current_alien_bullet;
   bunker_t *current_bunker;
   
   // iterate through the bunkers
   for (int i = 0; i < NUMBER_OF_BUNKERS; i++) {
        current_bunker = game_objects->bunkers[i];
        if (xSemaphoreTake(current_bunker->lock, 0) == pdTRUE) {    
            for (int k = 0; k < BUNKER_X_CELLS_NUMBER; k++) {  // itearte through the bunker cells
                for (int b = 0; b < MAX_ACTIVE_ALIEN_BULLETS; b++) { // iterate through all the alien bullets
                    current_alien_bullet = game_objects->alien_bullets[b];
                    if (xSemaphoreTake(current_alien_bullet->lock, 0) == pdTRUE) {
                        if (current_alien_bullet->active == OBJ_ACTIVE) {
                            if (current_bunker->upper_row[k].active == OBJ_ACTIVE) {
                                if (check_bunkercell_hit(&(current_bunker->upper_row[k]), current_alien_bullet)) {
                                    current_bunker->upper_row[k].active = OBJ_PASSIVE;
                                    current_alien_bullet->active = OBJ_PASSIVE;
                                }
                            }
                            if (current_bunker->middle_row[k].active == OBJ_ACTIVE) {
                                if (check_bunkercell_hit(&(current_bunker->middle_row[k]), current_alien_bullet)) {
                                    current_bunker->middle_row[k].active = OBJ_PASSIVE;
                                    current_alien_bullet->active = OBJ_PASSIVE;
                                }
                            }
                            if (current_bunker->lower_row[k].active == OBJ_ACTIVE) {
                                if (check_bunkercell_hit(&(current_bunker->lower_row[k]), current_alien_bullet)) {
                                    current_bunker->lower_row[k].active = OBJ_PASSIVE;
                                    current_alien_bullet->active = OBJ_PASSIVE;
                                }
                            }
                        }
                        xSemaphoreGive(current_alien_bullet->lock);
                    }
                }
                
                // check if hit by spaceship bullet
                current_alien_bullet = game_objects->my_bullet;
                if (xSemaphoreTake(current_alien_bullet->lock, 0) == pdTRUE) {
                    if (current_alien_bullet->active == OBJ_ACTIVE) {
                        if (current_bunker->upper_row[k].active == OBJ_ACTIVE) {
                            if (check_bunkercell_hit(&(current_bunker->upper_row[k]), current_alien_bullet)) {
                                current_bunker->upper_row[k].active = OBJ_PASSIVE;
                                current_alien_bullet->active = OBJ_PASSIVE;
                            }
                        }
                        if (current_bunker->middle_row[k].active == OBJ_ACTIVE) {
                            if (check_bunkercell_hit(&(current_bunker->middle_row[k]), current_alien_bullet)) {
                                current_bunker->middle_row[k].active = OBJ_PASSIVE;
                                current_alien_bullet->active = OBJ_PASSIVE;
                            }
                        }
                        if (current_bunker->lower_row[k].active == OBJ_ACTIVE) {
                            if (check_bunkercell_hit(&(current_bunker->lower_row[k]), current_alien_bullet)) {
                                current_bunker->lower_row[k].active = OBJ_PASSIVE;
                                current_alien_bullet->active = OBJ_PASSIVE;
                            }
                        }
                    }
                    xSemaphoreGive(current_alien_bullet->lock);
                }

            }
            xSemaphoreGive(current_bunker->lock);        
        }

   }

}

void vManageBunkersTask(game_objects_t *my_gameobjects){

    while (1) {
        BunkerCheckHit(my_gameobjects);
        
        vTaskDelay((TickType_t) (SCREEN_FREQUENCY));
    }
}

TaskHandle_t BunkersInitManageTask(game_objects_t *my_game_objects) {
    if (xTaskCreate(vManageBunkersTask, "ManageBunkersTask", mainGENERIC_STACK_SIZE * 2, my_game_objects,
                    mainGENERIC_PRIORITY, &ManageBunkersTask) != pdPASS) {
        printf("Failed to create Tast ManageBunkersTask");
        return NULL;
    }
    return ManageBunkersTask;
}
