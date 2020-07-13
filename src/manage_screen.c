#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Draw.h"

#include "my_structs.h"
#include "spaceship.h"
#include "bullet.h"
#include "alien.h"
#include "bunkers.h"


TaskHandle_t ManageScreenTask = NULL;

int DrawPregameMenu() {
    int x = SCREEN_WIDTH / 2 - 100;
    tumDrawText("[S]tart new game", x, 50, White);
    tumDrawText("[Q]uit", x, 100, White);
}

int DrawPauseMenu() {
    int x = SCREEN_WIDTH / 2 - 100;
    tumDrawText("PAUSED", x, 50, White);
    tumDrawText("[C]ontinue game", x, 100, White);
    tumDrawText("[R]estart game", x, 150, White);
    tumDrawText("[M] Return to main menu", x, SCREEN_HEIGHT - 100, White);
}

void vManageScreenTask(tasks_and_game_objects_t *tasks_and_game_objects){

    printf("Init Manage Screen");

    // init vars to update screen and call this task every .. milliseconds
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = SCREEN_FREQUENCY;
    

    // Needed such that Gfx library knows which thread controlls drawing
    // Only one thread can call tumDrawUpdateScreen while and thread can call
    // the drawing functions to draw objects. This is a limitation of the SDL
    // backend.
    tumDrawBindThread();

    xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.
    int current_game_state;

    while(1) {  //TODO: change this to while the screen is active
        vTaskDelayUntil( &xLastWakeTime, xFrequency );  // start this task every xFrequency millisecond

        if (xSemaphoreTake(tasks_and_game_objects->game_info->lock, 0) == pdTRUE) {
            current_game_state = tasks_and_game_objects->game_info->game_state;
            xSemaphoreGive(tasks_and_game_objects->game_info->lock);
        }


        tumDrawClear(Black); // Clear screen
        
        if (current_game_state == GAME_PRE_START) {
            DrawPregameMenu();
        } else if (current_game_state == GAME_PAUSED) {
            DrawPauseMenu();
        } else if (current_game_state == GAME_RUNNING) {
            SpaceShipDraw(tasks_and_game_objects->game_objects->my_spaceship);

            BulletDraw(tasks_and_game_objects->game_objects->my_bullet);

            BulletAlienDraw(tasks_and_game_objects->game_objects);

            AlienDrawMatrix(tasks_and_game_objects->game_objects);

            BunkersDraw(tasks_and_game_objects->game_objects);

            //draw score
            if (xSemaphoreTake(tasks_and_game_objects->game_objects->score->lock, 0) == pdTRUE) {
                char score_text[50]; 
                sprintf(score_text, "Score: %d   |   Highscore: %d   |   Level: %d   |   Lifes: %d   |   [P]ause game", 
                    tasks_and_game_objects->game_objects->score->current_score, tasks_and_game_objects->game_objects->score->highscore, tasks_and_game_objects->game_objects->score->level, tasks_and_game_objects->game_objects->score->lifes_left);
                tumDrawText(&score_text, 0, 0, White);
                xSemaphoreGive(tasks_and_game_objects->game_objects->score->lock);
            }   
        }        

        tumDrawUpdateScreen();
    }
    vTaskDelete(NULL); 
}

TaskHandle_t ManageGameScreenInit(tasks_and_game_objects_t *tasks_and_game_objects) {
    if (xTaskCreate(vManageScreenTask, "ManageScreenTask", mainGENERIC_STACK_SIZE * 2, tasks_and_game_objects,
                    mainGENERIC_PRIORITY, &ManageScreenTask) != pdPASS) {
        printf("Failed to create Tast MangaeButtonInit");
        return NULL;
    }
    return ManageScreenTask;
}
