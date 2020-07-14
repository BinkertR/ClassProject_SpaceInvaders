#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Font.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"

/*include SpaceInvaders specific modules*/
#include "my_structs.h"
#include "manage_screen.h"
#include "spaceship.h"
#include "bullet.h"
#include "manage_button_input.h"
#include "alien.h"
#include "bunkers.h"
#include "mothership.h"


#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)


int init_tum_lib(char *argv[]) {
    /* do the initialization needed to use the tumDraw, tumEvent, tumSound*/
    printf("Initializing: \n");

    char *bin_folder_path = tumUtilGetBinFolderPath(argv[0]);

    if (tumDrawInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize drawing");
        goto err_init_drawing;
    }

    if (tumEventInit()) {
        PRINT_ERROR("Failed to initialize events");
        goto err_init_events;
    }

    /*if (tumSoundInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize audio");
        goto err_init_audio;
    }*/

    return EXIT_SUCCESS;

    err_init_audio:
        tumEventExit();
    err_init_events:
        tumDrawExit();
    err_init_drawing:
        return EXIT_FAILURE;
}

int create_tasks() {
    // first create the game objects for all the tasks, so they can be passed to the tasks
    tasks_and_game_objects_t *tasks_and_game_objects = tasks_and_game_objects_init();
    
    //create all the tasks so they can be started by the scheduler

    tasks_and_game_objects->game_task_handlers->tasks[0] = BulletInitCalcTask(tasks_and_game_objects->game_objects);

    tasks_and_game_objects->game_task_handlers->tasks[1] = BunkersInitManageTask(tasks_and_game_objects->game_objects);

    tasks_and_game_objects->game_task_handlers->tasks[2] = AlienInitCalcMatrixTask(tasks_and_game_objects->game_objects);

    tasks_and_game_objects->game_task_handlers->tasks[3] = MotherShipInitCalcTask(tasks_and_game_objects->game_objects);

    tasks_and_game_objects->game_task_handlers->length = 4;

    // this task will be initialized with a higher priority since it is responsible for managing all the other task
    MangageButtonInit(tasks_and_game_objects);

    ManageGameScreenInit(tasks_and_game_objects);

    return EXIT_SUCCESS;

}

int main(int argc, char *argv[])
{   
    // TODO Give different task priorities
    if (init_tum_lib(argv) == EXIT_FAILURE) {return EXIT_FAILURE;}

    if (create_tasks() == EXIT_FAILURE) {return EXIT_FAILURE;}

    vTaskStartScheduler();

    return EXIT_SUCCESS;

}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vMainQueueSendPassed(void)
{
    /* This is just an example implementation of the "queue send" trace hook. */
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vApplicationIdleHook(void)
{
#ifdef __GCC_POSIX__
    struct timespec xTimeToSleep, xTimeSlept;
    /* Makes the process more agreeable when using the Posix simulator. */
    xTimeToSleep.tv_sec = 1;
    xTimeToSleep.tv_nsec = 0;
    nanosleep(&xTimeToSleep, &xTimeSlept);
#endif
}
