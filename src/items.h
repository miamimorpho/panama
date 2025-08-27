#include <stdint.h>
#include "dungeon_public.h"
#include "space.h"

typedef Handle Item;

struct ItemStats{
    int32_t attack;
};

struct ItemMan{
    struct MobileArray mobs;
    utf32_t *tiles;
    struct ItemStats *stats;
    char **names;
};

struct ItemMan *itemManCreate(void);
int itemCreate(struct Dungeon *d, vec16 ,char*, utf32_t, struct ItemStats, Item *);


