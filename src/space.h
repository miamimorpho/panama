#ifndef DUNGEON_MEM_H
#define DUNGEON_MEM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "dungeon_public.h"
#include "terminal.h"
#include "plist.h"
#include "maths.h"
#include "ivec16.h"

typedef enum {
    SPACE_OK = 0,
    SPACE_FAIL,
    SPACE_ERR,
}SpaceErr;

enum MobileType{
    MONSTERS,
    ITEMS,
    TYPE_COUNT,
};

struct Mobile{
    PLIST_ENTRY() link;
    union{
        vec16 pos;
        const vec16 *parent;
    };
};

struct MobileArray{
    Handle c;
    enum MobileType type;
    PLIST_HEAD() free;
    struct Mobile *root;
};

struct TerraPos{
    struct SpaceChunk *chunk;
    vec16 pos;
};

struct MobileFinder{
    struct Space *s;

    uint32_t cur_chunk;
    uint32_t total_chunks;
    uint32_t length;
    vec16 origin_chunk;

    struct Mobile *cur_mobile;
    struct MobileArray *arr;
};

struct Space * spaceCreate(void);

struct MobileArray mobileArrayCreate(size_t, enum MobileType);
SpaceErr mobileCreate(struct Space *, struct MobileArray*, vec16, Handle*);
SpaceErr mobileRemove(struct Space *, struct MobileArray*, Handle);
SpaceErr mobileMove(struct Space *, struct MobileArray*, Handle, vec16);
SpaceErr mobileGet(struct Space * , struct MobileArray*, vec16, Handle *);

struct Mobile* mobileFindStart(struct Space *, struct MobileArray *, vec16, uint32_t, struct MobileFinder *);
struct Mobile* mobileFindNext(struct MobileFinder *);
#define MOBILE_FIND(space, mobs, origin, range, finder) \
    for(struct Mobile* mob = mobileFindStart(space, mobs, origin, range, finder); \
            mob != NULL;    \
            mob = mobileFindNext(finder))   \

struct TerraPos terraPos(struct Space *, vec16, bool);
void terraPutOpaque(struct TerraPos, bool);
bool terraGetOpaque(struct TerraPos);
void terraPutSolid(struct TerraPos, bool);
bool terraGetSolid(struct TerraPos);
void terraPutTile(struct TerraPos , utf32_t);
utf32_t terraGetTile(struct TerraPos);

#endif // DUNGEON_MEM_H

