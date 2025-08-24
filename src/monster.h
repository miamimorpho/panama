#include <stdint.h>
#include "dungeon.h"
#include "ivec16.h"
#include "utf32.h"

#define ALIVE 1
#define DEAD 0

#define MONSTER_NULL UINT64_MAX

typedef uint64_t Monster;

struct Monsters *monsterManCreate(void);
Monster monsterCreate(struct Dungeon *d, vec16 p);
int monsterMove(struct Dungeon *d, Monster mon, vec16 next);
int monsterGet(struct Dungeon *d, vec16 in, Monster *out);
int monsterPos(struct Dungeon *d, Monster n, vec16 out);
utf32_t *monsterTile(struct Dungeon *d, Monster n);

Monster monsterEnd(struct Monsters *ctx, Monster n);
