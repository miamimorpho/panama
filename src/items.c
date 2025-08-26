#include <string.h>
#include "items.h"
#include "space.h"

#define ITEM_COUNT 128

struct ItemMan{
    struct MobileArray mobs;
    utf32_t *tiles;
    struct ItemStats *stats;
    char **names;
};

struct ItemMan *itemManCreate(void)
{
    struct ItemMan *m = malloc(sizeof(struct ItemMan));

    m->mobs = mobileArrayCreate(ITEM_COUNT, ITEMS);
    
    size_t tile_size =
        sizeOverflowCheck(ITEM_COUNT, sizeof(utf32_t));
    assert((m->tiles = malloc(tile_size)));
    memset(m->tiles, 0, tile_size);

    size_t stat_size =
        sizeOverflowCheck(ITEM_COUNT, sizeof(struct ItemStats));
    assert((m->stats = malloc(stat_size)));
    memset(m->stats, 0, stat_size);

    size_t name_size =
        sizeOverflowCheck(ITEM_COUNT, sizeof(char*));
    assert((m->names = malloc(name_size)));
    memset(m->names, 0, name_size);

    return m;
}

int 
itemCreate(struct Dungeon *d, 
        vec16 where,
        char* name,
        utf32_t tile,
        struct ItemStats stat,
        Item *out)
{
    assert(!mobileCreate(d->space, &d->items->mobs, where, out));
    Item i = *out;

    d->items->names[i] = calloc(strlen(name), sizeof(char));
    strncpy(d->items->names[i], name, strlen(name));

    d->items->tiles[i] = tile;
    d->items->stats[i] = stat;

    return 0;
}
