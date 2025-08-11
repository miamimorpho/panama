#include "render.h"
#include "terminal.h"

int drawMobs(struct Dungeon *d, uint32_t in_x, uint32_t in_y)
{
    termClear();

    uint16_t w, h;
    termSize(&w, &h);

    int32_t cam_x = in_x - (w / 2);
    int32_t cam_y = in_y - (h / 2);

    for(int y = 0; y < h; y++){
        for(int x = 0; x < w; x++){
            struct TerraPos pos = terraPos(d, cam_x + x, cam_y + y, 0);
            utf32_t ch = terraGetTile(pos);
            termCh((struct TermUI){x, y}, ch);
        }
    }

    struct Mob *mob;
    MOBS_FOREACH(d, mob, ALIVE){
        termCh((struct TermUI){mob->pos.x - cam_x, mob->pos.y - cam_y}, mob->ch);
    }

    return 0;
}
