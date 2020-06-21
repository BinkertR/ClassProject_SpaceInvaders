#include "TUM_Draw.h"

image_handle_t AlienLoadImg(int alien_score);

int AlienDrawSingle(alien_t *my_alien);

int AlienDrawColumn(alien_t *alien_column_start);

int AlienDrawMatrix(alien_t ***alien_matrix_start);

int AlienInitCalcSingleTask(game_objects_t *my_game_objects);

int AlienInitCalcMatrixTask(game_objects_t *my_game_objects);
