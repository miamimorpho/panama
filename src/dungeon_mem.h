#ifndef DUNGEON_MEM_H
#define DUNGEON_MEM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "terminal.h"
#include "plist.h"
#include "maths.h"
#include "ivec16.h"

typedef const vec16 *PosView;

typedef struct Dungeon Dungeon;

struct Mob{
    PLIST_ENTRY() link;
    vec16 pos;
    utf32_t ch;
};
PLIST_HEAD(MobHead);

struct DungeonChunk;
struct TerraPos{
    struct DungeonChunk *chunk;
    vec16 pos;
};

#define ALIVE 1
#define DEAD 0

struct Dungeon *dungeonCreate(void);

struct Mob* mobCreate(struct Dungeon *, vec16);
void mobRemove(struct Dungeon *, struct Mob *);
int mobMove(struct Dungeon *, struct Mob *, vec16);
struct Mob *mobArrFirst(struct Dungeon *, bool);
struct Mob *mobArrNext(struct Dungeon *, struct Mob *, bool);
#define MOBS_FOREACH(mm_, mob_, alive_)                 \
    for((mob_) = mobArrFirst((mm_), (alive_));          \
        (mob_);                                         \
        (mob_) = mobArrNext((mm_), (mob_), (alive_)))

struct TerraPos terraPos(struct Dungeon*, vec16, bool);
void terraPutOpaque(struct TerraPos, bool);
bool terraGetOpaque(struct TerraPos);
void terraPutSolid(struct TerraPos, bool);
bool terraGetSolid(struct TerraPos);
void terraPutTile(struct TerraPos , utf32_t);
utf32_t terraGetTile(struct TerraPos);

#endif // DUNGEON_MEM_H
