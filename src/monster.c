#include <stddef.h>
#include <string.h>
#include "monster.h"
#include "space.h"

#define MONSTER_C 128

struct Monsters *monsterManCreate(void)
{    
    struct Monsters *m = malloc(sizeof(struct Monsters));

    m->mobs = mobileArrayCreate(MONSTER_C, MONSTERS);

    size_t tile_size =
        sizeOverflowCheck(MONSTER_C, sizeof(utf32_t));
    assert((m->tiles = malloc(tile_size)));
    memset(m->tiles, 0, tile_size);

    return m;
}

int monsterCreate(struct Dungeon *d, vec16 where, Monster *out){
    return mobileCreate(d->space, &d->monsters->mobs, where, out);
}

int monsterMove(struct Dungeon *d, Monster mon, vec16 next){

    Monster mons;
    if(!mobileGet(d->space, &d->monsters->mobs, next, &mons)){
        return 1;
    }

    return mobileMove(d->space, &d->monsters->mobs, mon, next);
}

int monsterWhere(struct Dungeon *d, Monster n, vec16 out){
    if(n >= d->monsters->mobs.c) return 1;
    vec16Copy(d->monsters->mobs.root[n].pos, out);
    return 0;
}

utf32_t *
monsterTile(struct Dungeon *d, Monster n){
    if(n >= d->monsters->mobs.c) return NULL;
    return &d->monsters->tiles[n];
}
