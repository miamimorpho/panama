#ifndef DUNGEON_MEM_H
#define DUNGEON_MEM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "terminal.h"
#include "plist.h"
#include "maths.h"
#include "ivec16.h"

enum MobileType{
    MONSTERS,
    ITEMS,
    TYPE_COUNT,
};

struct Mobile{
    PLIST_ENTRY() link;
    vec16 pos;
};

struct MobileArray{
    enum MobileType type;
    struct Mobile *arr;
};

struct TerraPos{
    struct SpaceChunk *chunk;
    vec16 pos;
};

struct Space * spaceCreate(void);

struct MobileArray mobileArrayCreate(size_t, enum MobileType);
void mobileInsert(struct Space *, struct MobileArray, size_t, vec16);
void mobileRemove(struct Space *, struct MobileArray, size_t);
int mobileMove(struct Space *, struct MobileArray, size_t, vec16);
struct Mobile *mobileGet(struct Space * , struct MobileArray, vec16);

struct TerraPos terraPos(struct Space *, vec16, bool);
void terraPutOpaque(struct TerraPos, bool);
bool terraGetOpaque(struct TerraPos);
void terraPutSolid(struct TerraPos, bool);
bool terraGetSolid(struct TerraPos);
void terraPutTile(struct TerraPos , utf32_t);
utf32_t terraGetTile(struct TerraPos);

#endif // DUNGEON_MEM_H

