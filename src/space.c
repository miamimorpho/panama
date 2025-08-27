#include <string.h>
#include <stdlib.h>

#include "space.h"
#include "bitmap.h"
#include "arr.h"

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

    size_t chunk_arr_size = 0;
    d->chunks = arrMalloc(CHUNK_COUNT, sizeof(struct SpaceChunk), 
            &chunk_arr_size);

    d->chunk_c = CHUNK_COUNT;
    PLIST_INIT(&d->free_chunks); 

    size_t tile_chunk_size = arrOverflowCheck(CHUNK_LENGTH * CHUNK_LENGTH, sizeof(utf32_t));
    for(unsigned int i = 0; i < CHUNK_COUNT; i++){
        struct SpaceChunk *chunk = &d->chunks[i];
        *chunk = (struct SpaceChunk){0};
        chunk->solid = bitmapCreate(CHUNK_LENGTH, CHUNK_LENGTH);
        chunk->opaque = bitmapCreate(CHUNK_LENGTH, CHUNK_LENGTH);
        chunk->tiles = malloc(tile_chunk_size);
        memset(chunk->tiles, 0, tile_chunk_size);
        for(int n = 0; n < TYPE_COUNT; n++){
            PLIST_INIT(&chunk->heads[n]);
        }
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

struct TerraPos
terraPosNew(struct Space* d, vec16 p)
{
    struct TerraPos ter = terraPos(d, p);
    if(!ter.chunk){
        
        ter.chunk = PLIST_FIRST(d->chunks, &d->free_chunks);
        assert(ter.chunk && "terraPos try_new out of memory");
        PLIST_REMOVE_HEAD(d->chunks, &d->free_chunks, link);

        posToChunkPos(p, ter.chunk->pos);
        chunkInsert(d, ter.chunk);
        // TODO check save_data for record of chunk    
    }
    return ter;
}

struct TerraPos terraPos(struct Space *d, vec16 p)
{
    vec16 ch_p; 
    posToChunkPos(p, ch_p);

    return (struct TerraPos){
        .chunk = chunkGet(d, ch_p),
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
    size_t size;
    out.root = arrMalloc(c, sizeof(struct Mobile), &size); 
    out.c = c;
    out.type = type;

    for(unsigned int i = 0; i < c; i++){
        PLIST_INSERT_HEAD(out.root, &out.free, &out.root[i], link);
    }

    return out;
}

struct SpaceChunk *
mobileGetChunk(struct Space *d, vec16 p){
    vec16 chup;
    posToChunkPos(p, chup);
    return chunkGet(d, chup);
}

SpaceErr
mobileCreate(struct Space *s, struct MobileArray *arr, 
        vec16 where, Handle *i)
{
    struct Mobile *mob;
    if(!(mob = PLIST_FIRST(arr->root, &arr->free)))
        return SPACE_ERR;

    *i = mob - arr->root;
    PLIST_REMOVE(arr->root, &arr->free, mob, link);

    vec16Copy(where, mob->pos);
    struct SpaceChunk *chu = mobileGetChunk(s, where);
    if(!chu)
        return SPACE_FAIL;

    struct MobileHead *h = &chu->heads[arr->type];
    PLIST_INSERT_HEAD(arr->root, h, mob, link);

    return SPACE_OK;
}

SpaceErr
mobileRemove(struct Space *s, struct MobileArray *arr, Handle i){
    if(i >= arr->c){
        return SPACE_ERR;
    }
    struct Mobile *mob = &arr->root[i];
    struct SpaceChunk *chu = mobileGetChunk(s, mob->pos);
    struct MobileHead *h = &chu->heads[arr->type];
    PLIST_REMOVE(arr->root, h, mob, link);
    PLIST_INSERT_HEAD(arr->root, &arr->free, mob, link);

    return SPACE_OK;
}

SpaceErr 
mobileMove(struct Space *s, struct MobileArray *arr, Handle i, vec16 dest)
{
    if(terraGetSolid(terraPos(s, dest))){
        return SPACE_FAIL;
    }

    struct SpaceChunk *next_sc;
    if(!(next_sc = mobileGetChunk(s, dest)))
        return SPACE_FAIL;

    struct Mobile *mob = &arr->root[i];
    struct SpaceChunk *sc = mobileGetChunk(s, mob->pos);
    struct MobileHead *h = &sc->heads[arr->type];
    PLIST_REMOVE(arr->root, h, mob, link);

    vec16Copy(dest, mob->pos);
    h = &next_sc->heads[arr->type];
    PLIST_INSERT_HEAD(arr->root, h, mob, link);

    return SPACE_OK;
}

SpaceErr
mobileGet(struct Space *d, struct MobileArray *arr, vec16 p, Handle *out)
{
    struct SpaceChunk *sc = mobileGetChunk(d, p);
    if(!sc)
        return SPACE_FAIL;

    struct MobileHead *h = &sc->heads[arr->type];
    struct Mobile *mob;
    PLIST_FOREACH(arr->root, h, mob, link){
        if(vec16Equal(mob->pos, p)){
            *out = mob - arr->root;
            return SPACE_OK;
        }
    }

    return SPACE_FAIL;
}

static struct Mobile*
findFirstMobileInChunk(struct MobileFinder *find, uint32_t chunk_i)
{
    if(chunk_i >= find->total_chunks)
        return NULL;

    int32_t dx = chunk_i % find->length;
    int32_t dy = chunk_i / find->length;

    vec16 chunk_pos = {
        find->origin_chunk[0] - (find->length / 2) + dx,
        find->origin_chunk[1] - (find->length / 2) + dy,
    };

    struct SpaceChunk *chunk =
        chunkGet(find->s, chunk_pos);
    if(!chunk) return NULL;

    struct MobileHead *h =
        &chunk->heads[find->arr->type];

    struct Mobile *next;
    if((next = PLIST_FIRST(find->arr->root, h)))
        return next;
    else
        return NULL;
}

struct Mobile*
mobileFindNext(struct MobileFinder *find)
{
    if(find->cur_mobile){
        struct Mobile* next = 
            PLIST_NEXT(find->arr->root, find->cur_mobile, link);
        if(next) {
            find->cur_mobile = next;
            return next;
        }
        find->cur_mobile = NULL;
    }

    // move to next chunk
    find->cur_chunk++;

    while(find->cur_chunk < find->total_chunks){
        struct Mobile *first =
            findFirstMobileInChunk(find, find->cur_chunk);
        if(first){
            find->cur_mobile = first;
            return first;
        }
        find->cur_chunk++;
    }

    return NULL;
}

struct Mobile*
mobileFindStart(struct Space *s,
        struct MobileArray *arr,
        vec16 o,
        uint32_t range,
        struct MobileFinder *find)
{
    *find = (struct MobileFinder){0};
    find->arr = arr;
    find->s = s;
    posToChunkPos(o, find->origin_chunk);
    uint32_t radius = range / 2;
    find->length = 2 * radius + 1;
    find->total_chunks = find->length * find->length;
   
    find->cur_chunk = 0;
    find->cur_mobile = NULL;
    
    struct Mobile *first = findFirstMobileInChunk(find, 0);
    if(first){
        find->cur_mobile = first;
        return first;
    }
    
    return mobileFindNext(find);

}

