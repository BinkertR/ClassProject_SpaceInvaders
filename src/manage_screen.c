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
#include "mothership.h"

#define BACKGROUND_IMG "../img/nebula.jpg"

TaskHandle_t ManageScreenTask = NULL;

int DrawPregameMenu() {
    int x = SCREEN_WIDTH / 2 - 100;
    tumDrawText("[S]tart new game", x, 50, White);
    tumDrawText("[C]heat menu", x, 100, White);
    tumDrawText("[Q]uit", x, SCREEN_HEIGHT - 100, White);
}

int DrawPauseMenu() {
    int x = SCREEN_WIDTH / 2 - 100;
    tumDrawText("PAUSED", x, 50, White);
    tumDrawText("[C]ontinue game", x, 100, White);
    tumDrawText("[R]estart game", x, 150, White);
    tumDrawText("[M] Return to main menu", x, SCREEN_HEIGHT - 100, White);
}

int DrawGameEnded(tasks_and_game_objects_t *tasks_and_game_objects) {
    int x = SCREEN_WIDTH / 2 - 100;
    
    char score_text[50]; 
    
    if (xSemaphoreTake(tasks_and_game_objects->game_objects->score->lock, 0) == pdTRUE) {
        sprintf(score_text, " You reached Score: %d  and Level: %d", 
            tasks_and_game_objects->game_objects->score->current_score, tasks_and_game_objects->game_objects->score->level);
        tumDrawText(&score_text, x, 100, White);
        xSemaphoreGive(tasks_and_game_objects->game_objects->score->lock);
    }   

    tumDrawText("GAME ENDED", x, 50, White);

    tumDrawText("[M] Return to main menu", x, SCREEN_HEIGHT - 100, White);
}

int DrawCheatMenu(tasks_and_game_objects_t *tasks_and_game_objects) {
    int x = SCREEN_WIDTH / 2 - 100;

    tumDrawText("CHEAT MENU", x, 50, White);

    // draw state of infitive lifes cheat
    if (xSemaphoreTake(tasks_and_game_objects->game_objects->score->lock, 0) == pdTRUE) {
        if (tasks_and_game_objects->game_objects->score->infitive_lifes == 1) {
            tumDrawText("[I]nfinitve lifes: activated", x, 100, White);
        } else {
            tumDrawText("[I]nfinitve lifes: deactivated", x, 100, White);
        }
        xSemaphoreGive(tasks_and_game_objects->game_objects->score->lock);
    }

    tumDrawText("Hold UP/DOWN and press L/S to increase decrease Level/Score", x - 100, 150, White);
    if (xSemaphoreTake(tasks_and_game_objects->game_objects->score->lock, 0) == pdTRUE) {
            char LevelText[100];
            sprintf(LevelText, "[L] Current Starting level %d  (set in command line)", tasks_and_game_objects->game_objects->score->level);
            tumDrawText(LevelText, x, 200, White);
        xSemaphoreGive(tasks_and_game_objects->game_objects->score->lock);
    }
    if (xSemaphoreTake(tasks_and_game_objects->game_objects->score->lock, 0) == pdTRUE) {
            char LevelText[100];
            sprintf(LevelText, "[S] Current Starting Score %d  (set in command line)", tasks_and_game_objects->game_objects->score->current_score);
            tumDrawText(LevelText, x, 250, White);
        xSemaphoreGive(tasks_and_game_objects->game_objects->score->lock);
    }


    tumDrawText("[M] Return to main menu", x, SCREEN_HEIGHT - 100, White);
}

void vManageScreenTask(tasks_and_game_objects_t *tasks_and_game_objects){

    printf("Init Manage Screen");

    // init vars to update screen and call this task every .. milliseconds
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = SCREEN_FREQUENCY;
    
    // load background img

    image_handle_t background_img_handle = tumDrawLoadImage(BACKGROUND_IMG);
    tumDrawSetLoadedImageScale(background_img_handle, SCREEN_HEIGHT * 1.0 / tumDrawGetLoadedImageHeight(background_img_handle));

    // Needed such that Gfx library knows which thread controlls drawing
    // Only one thread can call tumDrawUpdateScreen while and thread can call
    // the drawing functions to draw objects. This is a limitation of the SDL
    // backend.
    tumDrawBindThread();

    xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.
    int current_game_state = GAME_PRE_START, current_highscore = 0;

    while(1) {  //TODO: change this to while the screen is active
        vTaskDelayUntil( &xLastWakeTime, xFrequency );  // start this task every xFrequency millisecond

        if (xSemaphoreTake(tasks_and_game_objects->game_info->lock, 0) == pdTRUE) {
            current_game_state = tasks_and_game_objects->game_info->game_state;
            xSemaphoreGive(tasks_and_game_objects->game_info->lock);
        }


        tumDrawClear(Black); // Clear screen
        tumDrawLoadedImage(background_img_handle, 0, 0);
        
        if (current_game_state == GAME_PRE_START) {
            DrawPregameMenu();
        } else if (current_game_state == GAME_PAUSED) {
            DrawPauseMenu();
        } else if (current_game_state == GAME_ENDED) {
            DrawGameEnded(tasks_and_game_objects);
        } else if (current_game_state == GAME_CHEAT_MENU) {
            DrawCheatMenu(tasks_and_game_objects);
        } else if (current_game_state == GAME_RUNNING) {
            SpaceShipDraw(tasks_and_game_objects->game_objects->my_spaceship);

            BulletDraw(tasks_and_game_objects->game_objects->my_bullet);

            BulletAlienDraw(tasks_and_game_objects->game_objects);

            AlienDrawMatrix(tasks_and_game_objects->game_objects);

            BunkersDraw(tasks_and_game_objects->game_objects);

            MothershipDraw(tasks_and_game_objects->game_objects);

            //draw score
            if (xSemaphoreTake(tasks_and_game_objects->game_info->lock, 0) == pdTRUE) {
                current_highscore = tasks_and_game_objects->game_info->highscore;
                xSemaphoreGive(tasks_and_game_objects->game_info->lock);
            }
            if (xSemaphoreTake(tasks_and_game_objects->game_objects->score->lock, 0) == pdTRUE) {
                char score_text[50]; 
                sprintf(score_text, "Score: %d   |   Highscore: %d   |   Level: %d   |   Lifes: %d   |   [P]ause game", 
                    tasks_and_game_objects->game_objects->score->current_score, current_highscore, tasks_and_game_objects->game_objects->score->level, tasks_and_game_objects->game_objects->score->lifes_left);
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
