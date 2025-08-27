#include <string.h>
#include "items.h"
#include "arr.h"

#define ITEM_COUNT 128


struct ItemMan *itemManCreate(void)
{
    struct ItemMan *m = malloc(sizeof(struct ItemMan));

    m->mobs = mobileArrayCreate(ITEM_COUNT, ITEMS);

    size_t tile_size;
    m->tiles = arrMalloc(ITEM_COUNT, sizeof(utf32_t), &tile_size);

    size_t stat_size;
    m->stats = arrMalloc(ITEM_COUNT, sizeof(struct ItemStats), &stat_size);

    size_t names_size;
    m->names = arrMalloc(ITEM_COUNT, sizeof(char*), &names_size);

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
