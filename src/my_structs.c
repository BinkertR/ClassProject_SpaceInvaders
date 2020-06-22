
#include "TUM_Draw.h"

#include "my_structs.h"
#include "alien.h"

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
    int x = 0;
    x = SCREEN_WIDTH / 2 - SHIP_WIDTH / 2;
    my_spaceship->position.x = x;
    my_spaceship->position.y = SHIP_Y_CO;
    my_spaceship->lifes = PLAYER_LIFES;
    my_spaceship->lock = xSemaphoreCreateMutex(); // Locking mechanism

    return my_spaceship;
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
    row->lock = xSemaphoreCreateMutex(); // Locking mechanism
    return row;
}

score_t *ScoreInit() {
    score_t *score = pvPortMalloc(sizeof(score_t));
    score->current_score = 0;
    score->highscore = 0;
    score->level = 1;
    score->lifes_left = PLAYER_LIFES;
    score->lock = xSemaphoreCreateMutex(); // Locking mechanism
    return score;
}

game_objects_t *game_objects_init() {
    game_objects_t *game_objects = pvPortMalloc(sizeof(game_objects_t));
    game_objects->my_spaceship = SpaceShipInit();
    game_objects->my_bullet = BulletInit(); 
    game_objects->alien_bullets = AlienBulletInit();
    game_objects->alien_matrix = AlienInitMatrix();
    game_objects->score = ScoreInit();
    game_objects->lock = xSemaphoreCreateMutex();
    return game_objects;
}