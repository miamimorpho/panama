#include <string.h>
#include <stdlib.h>

#include "dungeon_mem.h"
#include "bitmap.h"

#define CHUNK_LENGTH 16
#define TERRA_POS_NULL (struct TerraPos){0}

#define SPATIAL_HASH_COUNT 128
#define CHUNK_COUNT 64
#define MOBILE_COUNT 128

struct DungeonChunk{
    struct{
        PLIST_ENTRY();
        POSITION_FIELD;
    }pos;
    /* Terrain */
    Bitmap *solid;
    Bitmap *opaque;
    utf32_t *tiles;

    struct MobList mobs;
};
PLIST_HEAD(DungeonChunkList);

struct Dungeon{
    size_t spatial_hash_c;
    struct DungeonChunkList *spatial_hash;

    size_t chunk_c;
    struct DungeonChunkList free_chunks;
    struct DungeonChunk *chunks;

    size_t mob_c;
    Bitmap *mob_tombs;
    struct Mob *mobs;
};

struct Dungeon *dungeonCreate(void)
{
    struct Dungeon *d;
    assert(d = malloc(sizeof(struct Dungeon)));

    assert(IS_POWER_OF_TWO(SPATIAL_HASH_COUNT));
    size_t spatial_hash_size = 
        sizeOverflowCheck(SPATIAL_HASH_COUNT, sizeof(struct DungeonChunkList));

    assert((d->spatial_hash = malloc(spatial_hash_size)));
    d->spatial_hash_c = SPATIAL_HASH_COUNT;
    for(size_t i = 0; i < SPATIAL_HASH_COUNT; i++){
        PLIST_INIT(&d->spatial_hash[i]);
    }

    size_t chunk_arr_size = 
        sizeOverflowCheck(CHUNK_COUNT, sizeof(struct DungeonChunk));
    assert((d->chunks = malloc(chunk_arr_size)));
    memset(d->chunks, 0, chunk_arr_size);
    d->chunk_c = CHUNK_COUNT;
    PLIST_INIT(&d->free_chunks); 

    size_t tile_chunk_size = sizeOverflowCheck(CHUNK_LENGTH * CHUNK_LENGTH, sizeof(utf32_t));
    for(unsigned int i = 0; i < CHUNK_COUNT; i++){
        struct DungeonChunk *chunk = &d->chunks[i];
        *chunk = (struct DungeonChunk){0};
        chunk->solid = bitmapCreate(CHUNK_LENGTH, CHUNK_LENGTH);
        chunk->opaque = bitmapCreate(CHUNK_LENGTH, CHUNK_LENGTH);
        chunk->tiles = malloc(tile_chunk_size);
        memset(chunk->tiles, 0, tile_chunk_size);
        PLIST_INIT(&chunk->mobs);
        PLIST_INSERT_HEAD(d->chunks, &d->free_chunks, chunk, pos);
    }

    size_t mob_arr_size = 
        sizeOverflowCheck(MOBILE_COUNT, sizeof(struct Mob));
    assert((d->mobs = malloc(mob_arr_size)));
    memset(d->mobs, 0, mob_arr_size);
    d->mob_c = MOBILE_COUNT;
    d->mob_tombs = bitmapCreate(MOBILE_COUNT, 1);
    bitmapFill(d->mob_tombs, 0);

    return d;
}

static struct DungeonChunkList *dungeonChunkListGet(struct Dungeon *d, uint16_t x, uint16_t y)
{
    uint32_t key = ((uint32_t)y << 16) | (uint32_t)x;
    size_t i = MODULO_POWER_OF_TWO(hashFunction32(key), d->spatial_hash_c);  
    return &d->spatial_hash[i];
}

static struct DungeonChunk *dungeonChunkGet(struct Dungeon *d, uint16_t x, uint16_t y)
{
    struct DungeonChunkList *list = 
        dungeonChunkListGet(d, x, y);
    struct DungeonChunk *chunk;
    PLIST_FOREACH(d->chunks, list, chunk, pos){
        if(chunk->pos.x == x && chunk->pos.y == y)
            return chunk;
    }

    return NULL;
}

static void dungeonChunkInsert(struct Dungeon *d, struct DungeonChunk *chunk)
{
    assert(d && chunk);
    PLIST_INSERT_HEAD(d->chunks,
            dungeonChunkListGet(d, chunk->pos.x, chunk->pos.y),
            chunk, pos);
}

/*
static void dungeonChunkRemove(struct Dungeon *d, struct DungeonChunk *chunk)
{
    assert(d && chunk);
    PLIST_REMOVE(d->chunks,
            dungeonChunkListGet(d, chunk->pos.x, chunk->pos.y),
            chunk, pos);
}
*/

struct TerraPos terraPos(struct Dungeon *d, uint16_t x, uint16_t y, bool try_new)
{
    /*
    if(x < 0 || y < 0){
        return TERRA_POS_NULL;
    }
    */

    uint16_t chunk_x = x / CHUNK_LENGTH;
    uint16_t chunk_y = y / CHUNK_LENGTH;

    struct DungeonChunk *chunk = dungeonChunkGet(d, chunk_x, chunk_y);
    if(!chunk){
        
        if(try_new){
            chunk = PLIST_FIRST(d->chunks, &d->free_chunks);
            assert(chunk && "terraPos try_new out of memory");
            PLIST_REMOVE_HEAD(d->chunks, &d->free_chunks, pos);
            chunk->pos.x = chunk_x;
            chunk->pos.y = chunk_y;
            dungeonChunkInsert(d, chunk);
            // TODO check save_data for record of chunk
        }else{
            return TERRA_POS_NULL;
        }
    
    }
    
    return (struct TerraPos){
        .chunk = chunk,
        .x = x % CHUNK_LENGTH,
        .y = y % CHUNK_LENGTH,
    };
}

void terraPutOpaque(struct TerraPos p, bool val){
    if(!p.chunk) return;
    bitmapPutPx(p.chunk->opaque, p.x, p.y, val);
}

bool terraGetOpaque(struct TerraPos p){
    if(!p.chunk) return 1;
    return bitmapGetPx(p.chunk->opaque, p.x, p.y, 1);
}

void terraPutSolid(struct TerraPos p, bool val){
    if(!p.chunk) return;
    bitmapPutPx(p.chunk->solid, p.x, p.y, val);
}

bool terraGetSolid(struct TerraPos p){
    if(!p.chunk) return 1;
    return bitmapGetPx(p.chunk->solid, p.x, p.y, 1);
}

void terraPutTile(struct TerraPos p, utf32_t ch){
    if(!p.chunk) return;
    p.chunk->tiles[p.y * CHUNK_LENGTH + p.x] = ch;
}

utf32_t terraGetTile(struct TerraPos p){
    if(!p.chunk) return 'x';
    return p.chunk->tiles[p.y * CHUNK_LENGTH + p.x];
}

/* if alive true , finds next living mob in array
 * if false, finds next dead mob
 */
struct Mob *mobArrNext(struct Dungeon *d, struct Mob *n, bool alive){
    size_t i = n - d->mobs + 1;
    while(alive ^ bitmapGetPx(d->mob_tombs, i, 0, alive)) i++;
    return i < d->mob_c ? &d->mobs[i] : NULL;
}

struct Mob *mobArrFirst(struct Dungeon *d, bool alive){
    return mobArrNext(d, d->mobs - 1, alive);
}

static struct MobList* listOfMob(struct Dungeon* d, struct Mob* mob)
{
    return &dungeonChunkGet(d, mob->pos.x / CHUNK_LENGTH, mob->pos.y / CHUNK_LENGTH)->mobs;
}

struct Mob* mobCreate(struct Dungeon *d, uint16_t x, uint16_t y)
{
    struct Mob* mob;
    assert((mob = mobArrFirst(d, DEAD)));

    size_t i = mob - d->mobs; 
    bitmapPutPx(d->mob_tombs, i, 0, ALIVE);
    
    mob->pos.x = x;
    mob->pos.y = y;
    PLIST_INSERT_HEAD(d->mobs, listOfMob(d, mob), mob, pos);

    return mob;
}

void mobRemove(struct Dungeon *d, struct Mob *mob)
{
    size_t i = mob - d->mobs;
    bitmapPutPx(d->mob_tombs, i, 0, DEAD);
    PLIST_REMOVE(d->mobs, listOfMob(d, mob), mob, pos);
}

struct Mob *mobGet(struct Dungeon *d, uint16_t x, uint16_t y)
{
    struct MobList *list = &dungeonChunkGet(d, x / CHUNK_LENGTH, y / CHUNK_LENGTH)->mobs;
    struct Mob *mob;
    PLIST_FOREACH(d->mobs, list, mob, pos){
        if(mob->pos.x == x && mob->pos.y == y)
            return mob;
    }

    return NULL;
}

int mobMove(struct Dungeon *d, struct Mob *mob, int16_t dx, int16_t dy)
{
    if(terraGetSolid(terraPos(d, mob->pos.x +dx, mob->pos.y +dy, 1))){
        return 1;
    }

    if(mobGet(d, mob->pos.x +dx, mob->pos.y +dy)){
        return 1;
    }

    PLIST_REMOVE(d->mobs, listOfMob(d, mob), mob, pos);

    mob->pos.x += dx;
    mob->pos.y += dy;

    PLIST_INSERT_HEAD(d->mobs, listOfMob(d, mob), mob, pos);

    return 0;
}

