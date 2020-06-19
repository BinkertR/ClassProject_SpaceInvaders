#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Draw.h"

#include "spaceship.h"

#define SCREEN_FREQUENCY    1000/60

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

TaskHandle_t ManageScreenTask = NULL;

void vManageScreenTask(spaceship_t *my_spaceship){


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

        SpaceShipDraw(my_spaceship);

        tumDrawUpdateScreen();
    }
    vTaskDelete(NULL); 
}

int ManageScreenInit(spaceship_t *my_spaceship) {
    if (xTaskCreate(vManageScreenTask, "ManageScreenTask", mainGENERIC_STACK_SIZE * 2, my_spaceship,
                    mainGENERIC_PRIORITY, &ManageScreenTask) != pdPASS) {
        printf("Failed to create Tast MangaeButtonInit");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}
