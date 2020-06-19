#include <time.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"


void vManageScreenTask(void *pvParameters){

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 1000/60;
    int framerate = 0;
    char framerate_text[50];
    time_t lastWakeTime = clock();   

    xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.

    while(1) {  //TODO: change this to while the screen is active

    }
}