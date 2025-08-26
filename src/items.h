#include <stdint.h>
#include "dungeon_public.h"

typedef Handle Item;

struct ItemStats{
    int32_t attack;
};

struct ItemMan *itemManCreate(void);
int itemCreate(struct Dungeon *d, vec16 ,char*, utf32_t, struct ItemStats, Item *);


