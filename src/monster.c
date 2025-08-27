#include <stddef.h>
#include <string.h>
#include "monster.h"
#include "space.h"
#include "arr.h"
#include "reader.h"

#define MONSTER_C 128

struct Monsters *monsterManCreate(void)
{    
    struct Monsters *m = malloc(sizeof(struct Monsters));

    m->mobs = mobileArrayCreate(MONSTER_C, MONSTERS);

    size_t tile_size;
    m->tiles = 
        arrMalloc(MONSTER_C, sizeof(utf32_t), &tile_size);

    size_t names_size;
    m->names =
        arrMalloc(MONSTER_C, sizeof(char*), &names_size);

    size_t stats_size;
    m->stats =
        arrMalloc(MONSTER_C, sizeof(struct MonsterSheet), &stats_size);

    return m;
}

int monsterJSONRead(struct Monsters *m, const char* filename, Monster i)
{ 
    cJSON *root = readJson(filename);
    if (!root) return 1;

    size_t len = 0;
    readJsonCopyString(root, "name", NULL, &len);
    m->names[i] = calloc(len, sizeof(char));
    readJsonCopyString(root, "name", &m->names[i], &len);
    // tile
    readJsonCopyChar(root, "tile", &m->tiles[i]);

    readJsonCopyInt(root, "atk", &m->stats[i].atk);
    readJsonCopyInt(root, "def", &m->stats[i].def);
    readJsonCopyInt(root, "hp", &m->stats[i].hp);

    cJSON_Delete(root);
    return 0;
}

int monsterCreate(struct Dungeon *d, vec16 where, Monster *out){
    return mobileCreate(d->space, &d->monsters->mobs, where, out);
}

int monsterMove(struct Dungeon *d, Monster mon, vec16 next, Monster *dst){
    
    Monster tmp;
    if(!mobileGet(d->space, &d->monsters->mobs, next, &tmp)){
        if(dst)
            *dst = tmp;
        return 1;
    }

    return mobileMove(d->space, &d->monsters->mobs, mon, next);
}

void monsterUpdateHealth(struct Monsters *mons, Monster i){
    if(mons->stats[i].hp <= 0){
        mons->tiles[i] = 'x';
    }
}

int monsterAttack(struct Dungeon *d, Monster atk_idx, Monster def_idx){

    struct MonsterSheet *atk, *def;
    atk = &d->monsters->stats[atk_idx];
    def = &d->monsters->stats[def_idx];

    def->hp -= atk->atk;
    monsterUpdateHealth(d->monsters, def_idx); 

    return 0;
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
