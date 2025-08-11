#ifndef DUNGEON_MEM_H
#define DUNGEON_MEM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "terminal.h"
#include "plist.h"
#include "maths.h"

typedef struct Dungeon Dungeon;

struct DungeonChunk;
struct TerraPos{
    struct DungeonChunk *chunk;
    POSITION_FIELD;
};

struct Mob{
    struct{
        PLIST_ENTRY();
        POSITION_FIELD;
    }pos;
    char ch;
};
PLIST_HEAD(MobList);

#define ALIVE 1
#define DEAD 0

struct Dungeon *dungeonCreate(void);

struct Mob* mobCreate(struct Dungeon *, uint16_t, uint16_t);
void mobRemove(struct Dungeon *, struct Mob *);
int mobMove(struct Dungeon *, struct Mob *, int16_t, int16_t);
struct Mob *mobArrFirst(struct Dungeon *, bool);
struct Mob *mobArrNext(struct Dungeon *, struct Mob *, bool);
#define MOBS_FOREACH(mm_, mob_, alive_)                 \
    for((mob_) = mobArrFirst((mm_), (alive_));          \
        (mob_);                                         \
        (mob_) = mobArrNext((mm_), (mob_), (alive_)))

struct TerraPos terraPos(struct Dungeon*, uint16_t, uint16_t, bool);
void terraPutOpaque(struct TerraPos, bool);
bool terraGetOpaque(struct TerraPos);
void terraPutSolid(struct TerraPos, bool);
bool terraGetSolid(struct TerraPos);
void terraPutTile(struct TerraPos , utf32_t);
utf32_t terraGetTile(struct TerraPos);

#endif // DUNGEON_MEM_H
