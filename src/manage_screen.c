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


TaskHandle_t ManageScreenTask = NULL;

void vManageScreenTask(game_objects_t *my_gameobjects){

    printf("Init Manage Screen");

    // init vars to update screen and call this task every .. milliseconds
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = SCREEN_FREQUENCY;
    //int framerate = 0;
    //char framerate_text[50];
    //time_t lastWakeTime = clock(); 
    

    // Needed such that Gfx library knows which thread controlls drawing
    // Only one thread can call tumDrawUpdateScreen while and thread can call
    // the drawing functions to draw objects. This is a limitation of the SDL
    // backend.
    tumDrawBindThread();

    xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.

    while(1) {  //TODO: change this to while the screen is active
        vTaskDelayUntil( &xLastWakeTime, xFrequency );  // start this task every xFrequency millisecond
        tumDrawClear(Black); // Clear screen

        SpaceShipDraw(my_gameobjects->my_spaceship);

        BulletDraw(my_gameobjects->my_bullet);

        BulletAlienDraw(my_gameobjects);

        AlienDrawMatrix(my_gameobjects);

        //draw score
        if (xSemaphoreTake(my_gameobjects->score->lock, 0) == pdTRUE) {
            char score_text[50]; 
            sprintf(score_text, "Score: %d   |   Highscore: %d   |   Level: %d   |   Lifes: %d", 
                my_gameobjects->score->current_score, my_gameobjects->score->highscore, my_gameobjects->score->level, my_gameobjects->score->lifes_left);
            tumDrawText(&score_text, 0, 0, White);
            xSemaphoreGive(my_gameobjects->score->lock);
        }

        tumDrawUpdateScreen();
    }
    vTaskDelete(NULL); 
}

int ManageScreenInit(game_objects_t *my_game_objects) {
    if (xTaskCreate(vManageScreenTask, "ManageScreenTask", mainGENERIC_STACK_SIZE * 2, my_game_objects,
                    mainGENERIC_PRIORITY, &ManageScreenTask) != pdPASS) {
        printf("Failed to create Tast MangaeButtonInit");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}
