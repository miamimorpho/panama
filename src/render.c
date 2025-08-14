#include "render.h"
#include "terminal.h"

int drawMobs(struct Dungeon *d, vec16 o)
{
    termClear();

    uint16_t w, h;
    termSize(&w, &h);

    int32_t cam_x = o[0] - (w / 2);
    int32_t cam_y = o[1] - (h / 2);

    for(int y = 0; y < h; y++){
        for(int x = 0; x < w; x++){
            struct TerraPos pos = 
                terraPos(d, (vec16){cam_x + x, cam_y + y}, 0);
            utf32_t ch = terraGetTile(pos);
            termCh((struct TermUI){x, y}, ch);
        }
    }

    struct Mob *mob;
    MOBS_FOREACH(d, mob, ALIVE){
        termCh((struct TermUI){mob->pos[0] - cam_x, mob->pos[1] - cam_y}, mob->ch);
    }

    return 0;
}
