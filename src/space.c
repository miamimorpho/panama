#include <string.h>
#include <stdlib.h>

#include "space.h"
#include "bitmap.h"

#define CHUNK_LENGTH 16
#define TERRA_POS_NULL (struct TerraPos){0}

#define SPATIAL_HASH_C 128
#define CHUNK_COUNT 64

PLIST_HEAD(MobileHead);

struct SpaceChunk{

    PLIST_ENTRY() link;
    vec16 pos;

    /* Terrain */
    Bitmap *solid;
    Bitmap *opaque;
    utf32_t *tiles;

    struct MobileHead heads[TYPE_COUNT - 1];
};
PLIST_HEAD(SpaceChunkHead);

struct Space{
    size_t hash_c;
    struct SpaceChunkHead *hash;

    size_t chunk_c;
    struct SpaceChunkHead free_chunks;
    struct SpaceChunk *chunks;
};

struct Space *
spaceCreate(void)
{
    struct Space *d;
    assert(d = malloc(sizeof(struct Space)));

    assert(IS_POWER_OF_TWO(SPATIAL_HASH_C));
    d->hash = 
        calloc(SPATIAL_HASH_C, sizeof(struct SpaceChunkHead));
    d->hash_c = SPATIAL_HASH_C;
    assert(d->hash);
    for(size_t i = 0; i < SPATIAL_HASH_C; i++){
        PLIST_INIT(&d->hash[i]);
    }

    size_t chunk_arr_size = 
        sizeOverflowCheck(CHUNK_COUNT, sizeof(struct SpaceChunk));
    assert((d->chunks = malloc(chunk_arr_size)));
    memset(d->chunks, 0, chunk_arr_size);

    d->chunk_c = CHUNK_COUNT;
    PLIST_INIT(&d->free_chunks); 

    size_t tile_chunk_size = sizeOverflowCheck(CHUNK_LENGTH * CHUNK_LENGTH, sizeof(utf32_t));
    for(unsigned int i = 0; i < CHUNK_COUNT; i++){
        struct SpaceChunk *chunk = &d->chunks[i];
        *chunk = (struct SpaceChunk){0};
        chunk->solid = bitmapCreate(CHUNK_LENGTH, CHUNK_LENGTH);
        chunk->opaque = bitmapCreate(CHUNK_LENGTH, CHUNK_LENGTH);
        chunk->tiles = malloc(tile_chunk_size);
        memset(chunk->tiles, 0, tile_chunk_size);
        PLIST_INIT(&chunk->heads[0]);
        PLIST_INSERT_HEAD(d->chunks, &d->free_chunks, chunk, link);
    }


    return d;
}

static struct SpaceChunkHead *
chunkHeadGet(struct Space *d, vec16 p)
{
    uint32_t key; 
    vec16Handle(p, &key);
    size_t i = MODULO_POWER_OF_TWO(hashFunction32(key), d->hash_c);  
    return &d->hash[i];
}

static struct SpaceChunk *
chunkGet(struct Space *d, vec16 p)
{
    struct SpaceChunkHead *l = chunkHeadGet(d, p);
    struct SpaceChunk *ch;
    PLIST_FOREACH(d->chunks, l, ch, link){
        if(vec16Equal(ch->pos, p))
            return ch;
    }

    return NULL;
}

static void 
chunkInsert(struct Space *d, struct SpaceChunk *chunk)
{
    assert(d && chunk);
    struct SpaceChunkHead *h = chunkHeadGet(d, chunk->pos);
    PLIST_INSERT_HEAD(d->chunks, h, chunk, link);
}

/*
static void dungeonChunkRemove(struct DungeonLookup *d, struct DungeonChunk *chunk)
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


struct TerraPos terraPos(struct Space *d, vec16 p, bool try_new)
{
    vec16 ch_p; 
    posToChunkPos(p, ch_p);

    struct SpaceChunk *chunk = chunkGet(d, ch_p);
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

struct MobileArray
mobileArrayCreate(size_t c, enum MobileType type)
{

    struct MobileArray out = {0};

    size_t size = 
        sizeOverflowCheck(c, sizeof(struct Mobile));
    assert((out.arr = malloc(size)));
    memset(out.arr, 0, size);
    out.type = type;

    return out;
}

struct MobileHead *
mobileHeadGet(struct Space *d, enum MobileType type, vec16 p){
    vec16 ch_p;
    posToChunkPos(p, ch_p);
    return &chunkGet(d, ch_p)->heads[type];
}

void
mobileInsert(struct Space *s, struct MobileArray src, size_t i, vec16 to)
{    
    struct Mobile *mob = &src.arr[i];
    vec16Copy(to, mob->pos);
    struct MobileHead *h = mobileHeadGet(s, src.type, mob->pos);
    PLIST_INSERT_HEAD(src.arr, h, mob, link);
}

void
mobileRemove(struct Space *s, struct MobileArray src, size_t i){
    struct Mobile *mob = &src.arr[i];
    struct MobileHead *h = mobileHeadGet(s, src.type, mob->pos);
    PLIST_REMOVE(src.arr, h, mob, link);
}

int 
mobileMove(struct Space *s, struct MobileArray src, size_t i, vec16 dest)
{
    if(terraGetSolid(terraPos(s, dest, 1))){
        return 1;
    }

    struct Mobile *mob = &src.arr[i];
    struct MobileHead *h = 
        mobileHeadGet(s, src.type, mob->pos);
    PLIST_REMOVE(src.arr, h, mob, link);

    vec16Copy(dest, mob->pos);

    h = mobileHeadGet(s, src.type, mob->pos);
    PLIST_INSERT_HEAD(src.arr, h, mob, link);

    return 0;
}

struct Mobile *
mobileGet(struct Space *d, struct MobileArray src, vec16 p)
{
    struct MobileHead *h = 
        mobileHeadGet(d, src.type, p);
    struct Mobile *mob = NULL;
    PLIST_FOREACH(src.arr, h, mob, link){
        if(vec16Equal(mob->pos, p))
            return mob;
    }

    return NULL;
}
