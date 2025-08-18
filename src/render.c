#include "render.h"
#include "terminal.h"
#include "fov.h"

struct FovDrawCtx{
    vec16 offset;
};

void fovDrawFn(struct FovEffect *p, struct TerraPos ter, vec16 in)
{
    struct FovDrawCtx *ctx = (struct FovDrawCtx*)p->ctx;
    struct TermUI ui = { 
        .x = in[0] - ctx->offset[0],
        .y = in[1] - ctx->offset[1]
    };
    termCh(ui, terraGetTile(ter));
}

int drawDungeon(struct Dungeon *d, vec16 o)
{
    termClear();

    uint16_t w, h;
    termSize(&w, &h);

    int32_t cam_x = o[0] - (uint32_t)(w / 2);
    int32_t cam_y = o[1] - (uint32_t)(h / 2);

    struct FovDrawCtx ctx;
    vec16Copy((vec16){cam_x, cam_y}, ctx.offset);

    struct FovEffect draw = {
        .fn = fovDrawFn,
        .ctx = &ctx,
    };

    fov(d, o, &draw);

    /*
    for(int y = 0; y < h; y++){
        for(int x = 0; x < w; x++){
            struct TerraPos pos = 
                terraPos(d, (vec16){cam_x + x, cam_y + y}, 0);
            utf32_t ch = terraGetTile(pos);
            termCh((struct TermUI){x, y}, ch);
        }
    }
    */

    struct Mob *mob;
    MOBS_FOREACH(d, mob, ALIVE){
        termCh((struct TermUI){mob->pos[0] - cam_x, mob->pos[1] - cam_y}, mob->ch);
    }

    return 0;
}
