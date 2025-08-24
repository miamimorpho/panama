#include <stddef.h>
#include <string.h>
#include "monster.h"
#include "space.h"
#include "bitmap.h"

#define MONSTER_C 128

struct Monsters{ 
    size_t c;
    Bitmap *tombs;
    struct MobileArray mobs;
    utf32_t *tiles;
};

struct Monsters *monsterManCreate(void)
{    
    struct Monsters *m = malloc(sizeof(struct Monsters));

    m->mobs = mobileArrayCreate(MONSTER_C, MONSTERS);

    size_t tile_size =
        sizeOverflowCheck(MONSTER_C, sizeof(utf32_t));
    assert((m->tiles = malloc(tile_size)));
    memset(m->tiles, 0, tile_size);

    m->c = MONSTER_C;
    m->tombs = bitmapCreate(MONSTER_C, 1);
    bitmapFill(m->tombs, DEAD);

    return m;
}

Monster monsterNext(struct Monsters *m, Monster n, bool alive){
    Monster i = n;
    while(alive ^ bitmapGetPx(m->tombs, i, 0, alive)) i++;
    return i;
}

Monster monsterFirst(struct Monsters *m, bool alive){
    return monsterNext(m, 0, alive);
}

Monster monsterEnd(struct Monsters *ctx, Monster n){
    return n >= ctx->c;
}

Monster monsterCreate(struct Dungeon *d, vec16 p)
{
    Monster i = 0;
    i = monsterFirst(d->monsters, DEAD);
    if(monsterEnd(d->monsters, i)) return MONSTER_NULL;

    mobileInsert(d->space, d->monsters->mobs, i, p);
    bitmapPutPx(d->monsters->tombs, i, 0, ALIVE);
    return i;
}

void monsterRemove(struct Dungeon *d, Monster i)
{
    mobileRemove(d->space, d->monsters->mobs, i);
    bitmapPutPx(d->monsters->tombs, i, 0, DEAD);
}

int monsterGet(struct Dungeon *d, vec16 in, Monster *out){
    struct Mobile *tmp;
    if((tmp = mobileGet(d->space, d->monsters->mobs, in))){
        *out = tmp - d->monsters->mobs.arr;
        return 0;
    }
    *out = MONSTER_NULL;
    return 1;
}

int monsterMove(struct Dungeon *d, Monster mon, vec16 next){

    Monster mons;
    if(!monsterGet(d, next, &mons)){
        return 1;
    }

    return mobileMove(d->space, d->monsters->mobs, mon, next);
}

int monsterPos(struct Dungeon *d, Monster n, vec16 out){
    if(monsterEnd(d->monsters, n)) return 1;
    vec16Copy(d->monsters->mobs.arr[n].pos, out);
    return 0;
}

utf32_t *
monsterTile(struct Dungeon *d, Monster n){
    if(monsterEnd(d->monsters, n)) return NULL;
    return &d->monsters->tiles[n];
}
