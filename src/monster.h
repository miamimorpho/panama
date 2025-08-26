#include "space.h"

typedef Handle Monster;

struct Monsters{ 
    struct MobileArray mobs;
    utf32_t *tiles;
};

struct Monsters *monsterManCreate(void);
int monsterCreate(struct Dungeon *, vec16 , Monster*);
int monsterMove(struct Dungeon *, Monster, vec16);
int monsterGet(struct Dungeon *, vec16, Monster*);
utf32_t *monsterTile(struct Dungeon *d, Monster n);
int monsterWhere(struct Dungeon *d, Monster n, vec16 out);
