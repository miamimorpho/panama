#ifndef MONSTER_H
#define MONSTER_H

#include "space.h"

typedef Handle Monster;

struct MonsterSheet{
    int hp;
    int atk;
    int def;
};

struct Monsters{ 
    struct MobileArray mobs;
    utf32_t *tiles;
    struct MonsterSheet *stats;
    char **names;
};

struct Monsters *monsterManCreate(void);

int monsterCreate(struct Dungeon *, vec16 , Monster*);
int monsterJSONRead(struct Monsters *m, const char* filename, Monster i);

int monsterMove(struct Dungeon *, Monster, vec16, Monster *);
int monsterGet(struct Dungeon *, vec16, Monster*);
int monsterAttack(struct Dungeon *d, Monster atk_idx, Monster def_idx);

utf32_t *monsterTile(struct Dungeon *d, Monster n);
int monsterWhere(struct Dungeon *d, Monster n, vec16 out);

#endif //MONSTER_H
