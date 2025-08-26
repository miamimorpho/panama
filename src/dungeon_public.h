#ifndef DUNGEON_H
#define DUNGEON_H
#include "ivec16.h"
#include "utf32.h"

typedef uint64_t Handle;
static const Handle NULL_HANDLE = UINT64_MAX;

struct Dungeon{
    struct Space *space;
    struct Monsters *monsters;
    struct ItemMan *items;
};

void dungeonGenerate(struct Dungeon *);

#endif //DUNGEON_H
