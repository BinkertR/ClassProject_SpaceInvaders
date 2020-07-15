/*!
 * SpaceInvaders
 * @file mothership.c
 * @author Roman Binkert
 * @date 13 July 2020
 * @brief functions to display the mothership and communicate with the AI
*/

#include <stddef.h>
#include <stdio.h>
#include <string.h>


#include "TUM_Draw.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "AsyncIO.h"

#include "my_structs.h"

/* UDP Ports*/
#define UDP_BUFFER_SIZE 1024
#define UDP_RECEIVE_PORT 1234
#define UDP_TRANSMIT_PORT 1235

/* IMG */
#define MOTHERSHIP_IMG "../img/mothership.png"

TaskHandle_t MothershipCalcTask = NULL;
TaskHandle_t MothershipAITask = NULL;

image_handle_t MothershipLoadImg() {  
    /*!
    Loads and scales the png img according to the size of the motherhsip. 
        Store the img handle so it can be reused to display the mothership

    @return mothership_img: the imange_handle for the mothership.
    */
    image_handle_t mothership_img; 
    int img_width = 0;
    float scale_factor = 1.0;

    mothership_img = tumDrawLoadImage(MOTHERSHIP_IMG);  
    img_width = tumDrawGetLoadedImageWidth(mothership_img);
    scale_factor = MOTHERSHIP_WIDTH * 1.0 / img_width;
    tumDrawSetLoadedImageScale(mothership_img, scale_factor);  
    
    if (mothership_img == NULL) {
        printf("Failed to load mothership img");
        return NULL;
    }    
    return mothership_img;
}

int MothershipDraw(game_objects_t *game_objects) {
    coord_t mothership_draw_position;
    if (xSemaphoreTake(game_objects->mothership->lock, 0) == pdTRUE) {
        if (game_objects->mothership->active == OBJ_ACTIVE) {
            mothership_draw_position.x = game_objects->mothership->position.x - ALIEN_WIDTH / 2;
            mothership_draw_position.y = game_objects->mothership->position.y - MOTHERSHIP_HEIGHT / 2;
            tumDrawLoadedImage(game_objects->mothership->img_h, mothership_draw_position.x, mothership_draw_position.y);
        }
        xSemaphoreGive(game_objects->mothership->lock);   
    }     
    return 0;
}

int moveMothership(game_objects_t *game_objects) {
    /*!
    @brief moves the mothership into the received direction
    if the mothership goes out at one end of the screen it comes back in at the other end
    */
   if (xSemaphoreTake(game_objects->mothership->lock, 0) == pdTRUE) {
       int direction = game_objects->mothership->direction;
       game_objects->mothership->position.x += direction * MOTHERSHIP_SPEED;
       if (game_objects->mothership->position.x > SCREEN_WIDTH) {
           game_objects->mothership->position.x -= SCREEN_WIDTH;   // if mothership moves out of the screen on the right side, move it to the left side
       } else if (game_objects->mothership->position.x < 0) {
           game_objects->mothership->position.x += SCREEN_WIDTH;     // if mothership moves out of the screen on the left side, move it to the right side
       }
       xSemaphoreGive(game_objects->mothership->lock);
   }
    return 0;
}

void UDPHandler(size_t read_size, char *buffer, game_objects_t *game_objects) {
    int direction = NULL;
    if (strncmp(buffer, "INC", (read_size < 3) ? read_size : 3) == 0) {  // if opponent wants to increment position
        direction = 1;
    } else if (strncmp(buffer, "DEC", (read_size < 3) ? read_size : 3) == 0) {
        direction = -1;
    } else if (strncmp(buffer, "NONE", (read_size < 4) ? read_size : 4) == 0) {
        direction = 0;  /// is this HALT?
    } else if (strncmp(buffer, "HALT", (read_size < 4) ? read_size : 4) == 0) {
        direction = 0;
    }
    if (direction != NULL) {
        if (xSemaphoreTake(game_objects->mothership->lock, 0) == pdTRUE) {
            game_objects->mothership->direction = direction;
            xSemaphoreGive(game_objects->mothership->lock);
        }
    }
}

void vMothershipAITask(tasks_and_game_objects_t *tasks_and_game_objects)
{
    static char buf[50];
    char *addr = NULL; // Loopback
    in_port_t port = UDP_RECEIVE_PORT;
    char last_difficulty = -1;
    char difficulty = 1;
    int sent_pause = 0;  // should be 1 if pause was last sent and -1 if resume was last sent. Used to check if one of these messages needs to be send.
    aIO_handle_t udp_soc_receive;

    udp_soc_receive = aIOOpenUDPSocket(addr, port, UDP_BUFFER_SIZE, UDPHandler, tasks_and_game_objects->game_objects);

    printf("UDP socket opened on port %d\n", port);

    while (1) {
        int ai_task_activated = OBJ_PASSIVE;
        int mothership_active = OBJ_PASSIVE;
        if (xSemaphoreTake(tasks_and_game_objects->game_objects->mothership->lock, 0) == pdTRUE) {
            if (tasks_and_game_objects->game_objects->mothership->active == OBJ_ACTIVE) {
                mothership_active = OBJ_ACTIVE;
            }
            difficulty = tasks_and_game_objects->game_objects->mothership->ai_difficulty;
            xSemaphoreGive(tasks_and_game_objects->game_objects->mothership->lock);
        }
        // send pause/resume if the playmode changed
        if(xSemaphoreTake(tasks_and_game_objects->game_info->lock, 0) == pdTRUE) {
            if (tasks_and_game_objects->game_info->playmode == PLAYMODE_AI_PLAYER) {
                ai_task_activated = OBJ_ACTIVE;
            }
            if (tasks_and_game_objects->game_info->playmode == PLAYMODE_AI_PLAYER && mothership_active == OBJ_ACTIVE && sent_pause != -1) {
                // if the mothership is active but the resume msg is not sent yet, send it
                sprintf(buf, "RESUME");
                aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf, strlen(buf));
                sent_pause = -1;
            } else if ((tasks_and_game_objects->game_info->playmode == PLAYMODE_SINGEPLAYER || mothership_active == OBJ_PASSIVE) && sent_pause != 1) {
                // if the mothership is passive but the pause msg is not sent yet, send it
                sprintf(buf, "PAUSE");
                aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf, strlen(buf));
                sent_pause = 1;
            }
            xSemaphoreGive(tasks_and_game_objects->game_info->lock);
        }

        // only do this if the communication with the ai is currently active
        if (ai_task_activated == OBJ_ACTIVE & mothership_active == OBJ_ACTIVE) {
            // compute and send position difference to opponent
            signed int diff = 0;
            if (xSemaphoreTake(tasks_and_game_objects->game_objects->my_spaceship->lock, 0) == pdTRUE && xSemaphoreTake(tasks_and_game_objects->game_objects->mothership->lock, 0) == pdTRUE) {
                diff = tasks_and_game_objects->game_objects->my_spaceship->position.x - tasks_and_game_objects->game_objects->mothership->position.x;
                xSemaphoreGive(tasks_and_game_objects->game_objects->my_spaceship->lock);
                xSemaphoreGive(tasks_and_game_objects->game_objects->mothership->lock);
            }
            if (diff > 0) {
                sprintf(buf, "+%d", diff);
            }
            else {
                sprintf(buf, "-%d", -diff);
            }
            aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf, strlen(buf));

            // compute and send attacking/passive to opponent
            if (xSemaphoreTake(tasks_and_game_objects->game_objects->my_bullet->lock, 0) == pdTRUE) {
                if (tasks_and_game_objects->game_objects->my_bullet->active == OBJ_ACTIVE) {
                    sprintf(buf, "ATTACKING");
                } else {
                    sprintf(buf, "PASSIVE");
                }
                xSemaphoreGive(tasks_and_game_objects->game_objects->my_bullet->lock);
                aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf, strlen(buf));
            }

            // send the difficulty if it has changed
            if (last_difficulty != difficulty) {
                sprintf(buf, "D%d", difficulty);
                aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf, strlen(buf));
                last_difficulty = difficulty;
            }            
            moveMothership(tasks_and_game_objects->game_objects);
            
        }        
        vTaskDelay((TickType_t) (SCREEN_FREQUENCY));
    }
}

TaskHandle_t MothershipInitAITask(tasks_and_game_objects_t *tasks_and_game_objects) {
    /*! 
    @brief initialzie the vAlienCalcMatrixTask as a FreeRTOS Task
    */
    if (xTaskCreate(vMothershipAITask, "MothershipAI", mainGENERIC_STACK_SIZE * 2, tasks_and_game_objects,
                    mainGENERIC_PRIORITY, &MothershipAITask) != pdPASS) {
        printf("Failed to create Task MothershipAI");
        return NULL;
    }
    return MothershipAITask;
}


// functions for singleplayer

int GetRandIntMothership() {
    // get a random number between 0 and RAND_SHOOT_CHANCE
    int random_int;

    random_int = (rand() % (MOTHERHSIP_APPERANCE_CHANCE - 0 + 1)) + 0;

    return random_int;
}

void vMothershipCalcTask(game_objects_t *game_objects) {
    while (1)
    {
        if (xSemaphoreTake(game_objects->mothership->lock, 0) == pdTRUE) {
            // if mothership is passive give it a chance to get active            
            if (game_objects->mothership->active == OBJ_PASSIVE) {
                int r = GetRandIntMothership();
                if (r <= 1) {
                    game_objects->mothership->active = OBJ_ACTIVE;
                    game_objects->mothership->position.x = 0;  // TODO also start from the right side
                }
            }
            if (game_objects->mothership->active == OBJ_ACTIVE) {
                game_objects->mothership->position.x += MOTHERSHIP_SPEED;
                if (game_objects->mothership->position.x > SCREEN_WIDTH) {
                    game_objects->mothership->active = OBJ_PASSIVE;
                }
            }
            xSemaphoreGive(game_objects->mothership->lock);
        } 
        vTaskDelay((TickType_t) (SCREEN_FREQUENCY));
    }    
}

TaskHandle_t MotherShipInitCalcTask(game_objects_t *game_objects) {
    if (xTaskCreate(vMothershipCalcTask, "vMothershipCalcTask", mainGENERIC_STACK_SIZE * 2, game_objects,
                    mainGENERIC_PRIORITY, &MothershipCalcTask) != pdPASS) {
        printf("Failed to create Task MothershipCalcTask");
        return NULL;
    }
    return MothershipCalcTask;
}
