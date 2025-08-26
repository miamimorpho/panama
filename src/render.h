#include <stdint.h>
#include "dungeon_public.h"
#include "ivec16.h"

struct Camera{
    int32_t old_x;
    int32_t old_y;
    int32_t cam_x;
    int32_t cam_y;
    int16_t cursor_x;
    int16_t cursor_y;
    int32_t view_width;
    int32_t view_height;
};

int drawDungeon(struct Dungeon*, vec16);
