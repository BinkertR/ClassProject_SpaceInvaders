#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Draw.h"

#include "spaceship.h"

void vCalcSpaceShipTask(void *pvParameters){

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = SCREEN_FREQUENCY;
    int framerate = 0;
    char framerate_text[50];
    time_t lastWakeTime = clock();   
    

    printf("Init Manage Screen");

    // Needed such that Gfx library knows which thread controlls drawing
    // Only one thread can call tumDrawUpdateScreen while and thread can call
    // the drawing functions to draw objects. This is a limitation of the SDL
    // backend.
    tumDrawBindThread();

    xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.

    while(1) {  //TODO: change this to while the screen is active
        vTaskDelayUntil( &xLastWakeTime, xFrequency );  // start this task every xFrequency millisecond

        
    }
    vTaskDelete(NULL); 
}


int SpaceShipDraw(spaceship_t *spaceship) {
    // draw the spaceship according to its current position
    tumDrawFilledBox(0, 0, 10, 10, Green);
    return 0;
}
