#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Draw.h"

#include "my_structs.h"
#include "alien.h"      // to use load alien img function
#include "mothership.h"  // to use load mothership img function

bullet_t *BulletInit() {
    bullet_t *my_bullet = pvPortMalloc(sizeof(bullet_t));
    my_bullet->position.x = 0;
    my_bullet->position.y = 0;
    my_bullet->active = OBJ_PASSIVE;
    my_bullet->lock = xSemaphoreCreateMutex(); // Locking mechanism

    return my_bullet;
}

bullet_t **AlienBulletInit() {
    bullet_t **first_bullet = pvPortMalloc(sizeof(bullet_t) * MAX_ACTIVE_ALIEN_BULLETS);
    for (int i = 0; i < MAX_ACTIVE_ALIEN_BULLETS; i++) {
        first_bullet[i] = BulletInit();
    }
    return first_bullet;
}

spaceship_t *SpaceShipInit() {
    spaceship_t *my_spaceship = pvPortMalloc(sizeof(spaceship_t));
    my_spaceship->position.x = SCREEN_WIDTH / 2 - SHIP_WIDTH / 2;
    my_spaceship->position.y = SHIP_Y_CO;
    my_spaceship->lifes = PLAYER_LIFES;
    my_spaceship->lock = xSemaphoreCreateMutex(); // Locking mechanism

    return my_spaceship;
}

mothership_t *MothershipInit() {
    mothership_t *mothership = pvPortMalloc(sizeof(mothership_t));
    mothership->position.x = SCREEN_WIDTH / 2 - MOTHERSHIP_WIDTH / 2;
    mothership->position.y = MOTHERSHIP_Y_CO;
    mothership->img_h = MothershipLoadImg();
    mothership->active = 1;
    mothership->lock = xSemaphoreCreateMutex(); // Locking mechanism
    return mothership;
}

alien_t *AlienInit(int alien_score) {
    alien_t *my_alien = pvPortMalloc(sizeof(alien_t));
    my_alien->position.x = ALIEN_START_X;
    my_alien->position.y = ALIEN_START_Y;
    my_alien->alien_score = alien_score;
    my_alien->active = OBJ_ACTIVE;  // init to active because it should be display from the beginning
    my_alien->img_h = AlienLoadImg(alien_score);
    my_alien->lock = xSemaphoreCreateMutex(); // Locking mechanism
    return my_alien;
}

alien_column_t *AlienInitColumn(int x_position) {
    // initialize a column with 5 aliens and return the pointer to the first object.
    alien_column_t *column = pvPortMalloc(sizeof(alien_column_t));
    column->first_alien = pvPortMalloc(sizeof(alien_t) * ALIENS_PER_COLUMN);

    column->first_alien[0] = AlienInit(ALIEN_HARD);
    column->first_alien[1] = AlienInit(ALIEN_MIDDLE);
    column->first_alien[2] = AlienInit(ALIEN_MIDDLE);
    column->first_alien[3] = AlienInit(ALIEN_EASY);
    column->first_alien[4] = AlienInit(ALIEN_EASY);

    int *active_aliens = pvPortMalloc(sizeof(int) * ALIENS_PER_COLUMN);
    column->lowest_active_alien = ALIENS_PER_COLUMN - 1;
    column->active = OBJ_ACTIVE;
    
    for (int i = 0; i < ALIENS_PER_COLUMN; i++) {
        column->first_alien[i]->position.x = x_position;
        column->first_alien[i]->position.y += (ALIEN_WIDTH + ALIEN_PADDING_Y) * i;
        active_aliens[i] = OBJ_ACTIVE;
    }
    column->active_aliens = &(active_aliens[0]);
    column->lock = xSemaphoreCreateMutex(); // Locking mechanism
    return column;

}

alien_matrix_t *AlienInitMatrix() {
    alien_matrix_t *row = pvPortMalloc(sizeof(alien_matrix_t));
    row->first_column = pvPortMalloc(sizeof(alien_column_t) * sizeof(alien_t) * ALIENS_PER_ROW * ALIENS_PER_COLUMN);
    int x_position = 0;
    for (int i = 0; i < ALIENS_PER_ROW; i++) {  //sizeof(row) / sizeof(alien_t)
        x_position = ALIEN_START_Y + (ALIEN_WIDTH + ALIEN_PADDING_X) * i;
        row->first_column[i] = AlienInitColumn(x_position);
    }
    int *active_colums = pvPortMalloc(sizeof(int) * ALIENS_PER_ROW);
    for (int i = 0; i < ALIENS_PER_ROW; i++) {
        active_colums[i] = OBJ_ACTIVE;
    }
    row->active_columns = &(active_colums[0]);
    row->leftest_active_column = 0;
    row->rightest_active_column = ALIENS_PER_ROW - 1;
    row->killed_aliens_in_stage = 0;
    row->lock = xSemaphoreCreateMutex(); // Locking mechanism
    return row;
}

score_t *ScoreInit() {
    score_t *score = pvPortMalloc(sizeof(score_t));
    score->current_score = 0;
    score->level = 1;
    score->lifes_left = PLAYER_LIFES;
    score->infitive_lifes = 0;
    score->lock = xSemaphoreCreateMutex(); // Locking mechanism
    return score;
}

bunker_t **BunkerInit() {
    /*
    init the bunkers as an array and return the pointer to the first bunker.
    */

    bunker_t **bunker = pvPortMalloc(sizeof(bunker_t *) * NUMBER_OF_BUNKERS);
    
    for (int i = 0; i < NUMBER_OF_BUNKERS; i++) {
        bunker[i] = pvPortMalloc(sizeof(bunker_t));

        bunker[i]->position.x = SCREEN_WIDTH / NUMBER_OF_BUNKERS * i + BUNKER_CELL_SIZE * BUNKER_X_CELLS_NUMBER;
        bunker[i]->position.y = BUNKER_POSITION_Y; 
        bunker[i]->upper_row = pvPortMalloc(sizeof(bunker_cell_t) * BUNKER_X_CELLS_NUMBER);
        bunker[i]->middle_row = pvPortMalloc(sizeof(bunker_cell_t) * BUNKER_X_CELLS_NUMBER);
        bunker[i]->lower_row = pvPortMalloc(sizeof(bunker_cell_t) * BUNKER_X_CELLS_NUMBER);

        for (int k = 0; k < BUNKER_X_CELLS_NUMBER; k++) {
            // init all cells of the upper row
            bunker[i]->upper_row[k].active = OBJ_ACTIVE;
            bunker[i]->upper_row[k].position.x = bunker[i]->position.x + k * BUNKER_CELL_SIZE;
            bunker[i]->upper_row[k].position.y = bunker[i]->position.y;

            // init all cells of the middle row as passive (only the leftest and the rightes cell will be set to active after this loop)
            bunker[i]->middle_row[k].active = OBJ_PASSIVE;
            bunker[i]->middle_row[k].position.x = bunker[i]->position.x + k * BUNKER_CELL_SIZE;
            bunker[i]->middle_row[k].position.y = bunker[i]->position.y + BUNKER_CELL_SIZE;

            // init all cells of the lower row as active
            bunker[i]->lower_row[k].active = OBJ_ACTIVE;
            bunker[i]->lower_row[k].position.x = bunker[i]->position.x + k * BUNKER_CELL_SIZE;
            bunker[i]->lower_row[k].position.y = bunker[i]->position.y + BUNKER_CELL_SIZE * 2;

        }
        // set only the two outside bunker cells to active to form a hole in the middle of the bunker
        bunker[i]->middle_row[0].active = OBJ_ACTIVE;
        bunker[i]->middle_row[BUNKER_X_CELLS_NUMBER - 1].active = OBJ_ACTIVE;

        bunker[i]->lock = xSemaphoreCreateMutex(); // Locking mechanism         
    }
    return bunker;
}

int game_objects_init(game_objects_t *game_objects) {

    game_objects->my_spaceship = SpaceShipInit();
    game_objects->mothership = MothershipInit();
    game_objects->my_bullet = BulletInit(); 
    game_objects->alien_bullets = AlienBulletInit();
    game_objects->alien_matrix = AlienInitMatrix();
    game_objects->score = ScoreInit();
    game_objects->bunkers = BunkerInit();
    game_objects->lock = xSemaphoreCreateMutex();
    return 0;
}

tasks_and_game_objects_t *tasks_and_game_objects_init() {
    tasks_and_game_objects_t *tasks_and_game_objects = pvPortMalloc(sizeof(tasks_and_game_objects_t));
    tasks_and_game_objects->game_task_handlers = pvPortMalloc(sizeof(taskhandle_array_t));
    tasks_and_game_objects->game_task_handlers->tasks = pvPortMalloc(sizeof(TaskHandle_t) * 4);
    tasks_and_game_objects->game_info = pvPortMalloc(sizeof(game_info_t));
    tasks_and_game_objects->game_objects = pvPortMalloc(sizeof(game_objects_t));
    game_objects_init(tasks_and_game_objects->game_objects);

    tasks_and_game_objects->game_info->game_state = GAME_PRE_START;
    tasks_and_game_objects->game_info->highscore = 0;
    tasks_and_game_objects->game_info->lock = xSemaphoreCreateMutex();

    return tasks_and_game_objects;
}
