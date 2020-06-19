#include <stdio.h>

#include "TUM_Draw.h"

#include "my_structs.h"

image_handle_t AlienGreenLoadImg() {
    image_handle_t green_alien_img;
    int img_width = 0;
    float scale_factor = 1.0;
    green_alien_img = tumDrawLoadImage("../img/alien_green.png");
    if (green_alien_img == -1) {
        printf("Failed to load alien_green.png");
        return 1;
    }
    img_width = tumDrawGetLoadedImageWidth(green_alien_img);
    scale_factor = ALIEN_WIDTH * 1.0 / img_width;
    tumDrawSetLoadedImageScale(green_alien_img, scale_factor);
    return green_alien_img;
}

int AlienDrawSingle(image_handle_t alien_img_handle) {
    tumDrawLoadedImage(alien_img_handle, 0, 0);
    return 0;
}