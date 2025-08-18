#include <string.h>
#include <stdlib.h>

#include "dungeon_mem.h"
#include "bitmap.h"

#define CHUNK_LENGTH 16
#define TERRA_POS_NULL (struct TerraPos){0}

#define SPATIAL_HASH_C 128
#define CHUNK_COUNT 64
#define MOBILE_COUNT 128

struct DungeonChunk{

    PLIST_ENTRY() link;
    vec16 pos;

    /* Terrain */
    Bitmap *solid;
    Bitmap *opaque;
    utf32_t *tiles;

    struct MobHead mobs;
};
PLIST_HEAD(DungeonChunkHead);

struct Dungeon{
    size_t spatial_hash_c;
    struct DungeonChunkHead *spatial_hash;

    size_t chunk_c;
    struct DungeonChunkHead free_chunks;
    struct DungeonChunk *chunks;

    size_t mob_c;
    Bitmap *mob_tombs;
    struct Mob *mobs;
};

struct Dungeon *dungeonCreate(void)
{
    struct Dungeon *d;
    assert(d = malloc(sizeof(struct Dungeon)));

    assert(IS_POWER_OF_TWO(SPATIAL_HASH_C));
    d->spatial_hash = 
        calloc(SPATIAL_HASH_C, sizeof(struct DungeonChunkHead));
    d->spatial_hash_c = SPATIAL_HASH_C;
    assert(d->spatial_hash);
    for(size_t i = 0; i < SPATIAL_HASH_C; i++){
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
        PLIST_INSERT_HEAD(d->chunks, &d->free_chunks, chunk, link);
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

static struct DungeonChunkHead *
chunkHeadGet(struct Dungeon *d, vec16 p)
{
    uint32_t key; 
    vec16Handle(p, &key);
    size_t i = MODULO_POWER_OF_TWO(hashFunction32(key), d->spatial_hash_c);  
    return &d->spatial_hash[i];
}

static struct DungeonChunk *
chunkGet(struct Dungeon *d, vec16 p)
{
    struct DungeonChunkHead *l = chunkHeadGet(d, p);
    struct DungeonChunk *ch;
    PLIST_FOREACH(d->chunks, l, ch, link){
        if(vec16Equal(ch->pos, p))
            return ch;
    }

    return NULL;
}

static void 
chunkInsert(struct Dungeon *d, struct DungeonChunk *chunk)
{
    assert(d && chunk);
    struct DungeonChunkHead *h = chunkHeadGet(d, chunk->pos);
    PLIST_INSERT_HEAD(d->chunks, h, chunk, link);
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

static inline void 
posToChunkPos(vec16 mob, vec16 out)
{
    out[0] = floorDiv(mob[0], CHUNK_LENGTH);
    out[1] = floorDiv(mob[1], CHUNK_LENGTH);
}

struct MobHead *mobHeadGet(struct Dungeon *d, vec16 p){
    vec16 ch_p;
    posToChunkPos(p, ch_p);
    return &chunkGet(d, ch_p)->mobs;
}

struct TerraPos terraPos(struct Dungeon *d, vec16 p, bool try_new)
{
    vec16 ch_p; 
    posToChunkPos(p, ch_p);

    struct DungeonChunk *chunk = chunkGet(d, ch_p);
    if(!chunk){
        
        if(try_new){
            chunk = PLIST_FIRST(d->chunks, &d->free_chunks);
            assert(chunk && "terraPos try_new out of memory");
            PLIST_REMOVE_HEAD(d->chunks, &d->free_chunks, link);
            vec16Copy(ch_p, chunk->pos);
            chunkInsert(d, chunk);
            // TODO check save_data for record of chunk
        }else{
            return TERRA_POS_NULL;
        }
    
    }
    
    return (struct TerraPos){
        .chunk = chunk,
        .pos[0] = floorMod(p[0], CHUNK_LENGTH),
        .pos[1] = floorMod(p[1], CHUNK_LENGTH)
    };
}

void terraPutOpaque(struct TerraPos p, bool val){
    if(!p.chunk) return;
    bitmapPutPx(p.chunk->opaque, p.pos[0], p.pos[1], val);
}

bool terraGetOpaque(struct TerraPos p){
    if(!p.chunk) return 1;
    return bitmapGetPx(p.chunk->opaque, p.pos[0], p.pos[1], 1);
}

void terraPutSolid(struct TerraPos p, bool val){
    if(!p.chunk) return;
    bitmapPutPx(p.chunk->solid, p.pos[0], p.pos[1], val);
}

bool terraGetSolid(struct TerraPos p){
    if(!p.chunk) return 1;
    return bitmapGetPx(p.chunk->solid, p.pos[0], p.pos[1], 1);
}

void terraPutTile(struct TerraPos p, utf32_t ch){
    if(!p.chunk) return;
    p.chunk->tiles[p.pos[1] * CHUNK_LENGTH + p.pos[0]] = ch;
}

utf32_t terraGetTile(struct TerraPos p){
    if(!p.chunk) return 'x';
    return p.chunk->tiles[p.pos[1] * CHUNK_LENGTH + p.pos[0] ];
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


struct Mob* mobCreate(struct Dungeon *d, vec16 p)
{
    struct Mob* mob;
    assert((mob = mobArrFirst(d, DEAD)));

    size_t i = mob - d->mobs; 
    bitmapPutPx(d->mob_tombs, i, 0, ALIVE);
   
    vec16Copy(p, mob->pos);
    struct MobHead *h = mobHeadGet(d, mob->pos);
    PLIST_INSERT_HEAD(d->mobs, h, mob, link);

    return mob;
}

void mobRemove(struct Dungeon *d, struct Mob *mob)
{
    size_t i = mob - d->mobs;
    bitmapPutPx(d->mob_tombs, i, 0, DEAD);
    struct MobHead *h = mobHeadGet(d, mob->pos);
    PLIST_REMOVE(d->mobs, h, mob, link);
}

struct Mob *mobGet(struct Dungeon *d, vec16 p)
{
    struct MobHead *h = mobHeadGet(d, p);
    struct Mob *mob = NULL;
    PLIST_FOREACH(d->mobs, h, mob, link){
        if(vec16Equal(mob->pos, p))
            return mob;
    }

    return NULL;
}

int mobMove(struct Dungeon *d, struct Mob *mob, vec16 dest)
{
    if(terraGetSolid(terraPos(d, dest, 1))){
        return 1;
    }
    if(mobGet(d, dest)){
        return 1;
    }

    struct MobHead *h = mobHeadGet(d, mob->pos);
    PLIST_REMOVE(d->mobs, h, mob, link);

    vec16Copy(dest, mob->pos);

    h = mobHeadGet(d, mob->pos);
    PLIST_INSERT_HEAD(d->mobs, h, mob, link);

    return 0;
}

