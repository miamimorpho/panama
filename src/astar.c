#include <stdint.h>
#include <stdlib.h>
#include "astar.h"
#include "maths.h"

enum State{
    EMPTY_HASH,
    CLOSED,
    OPEN,
};

struct AStarNode{
    double h;
    double g;
    vec16 pos;
    vec16 parent;
    uint32_t heap_i;
    enum State state;
};

struct AStar{

    vec16 start;
    vec16 goal;

    uint32_t hash_max;
    uint32_t hash_c;
    uint32_t heap_max;
    uint32_t heap_top;
   
    uint32_t *heap;
    struct AStarNode *hash;
};

struct AStar *pQueCreate(void){

    struct AStar *q = malloc(sizeof(struct AStar));
    *q = (struct AStar){0};
    q->hash_max = 1024 * 16;
    q->heap_max = 1024 * 16;
    q->heap = calloc(q->heap_max, sizeof(uint32_t));
    q->hash = calloc(q->hash_max, sizeof(struct AStarNode));
    for(uint32_t i = 0; i < q->hash_max; i++){
        q->hash[i].state = EMPTY_HASH;
    }

    return q;
}

static inline uint32_t
hashFn(struct AStar *a, vec16 pos)
{
    uint32_t key;
    vec16Handle(pos, &key);
    return MODULO_POWER_OF_TWO(hashFunction32(key), a->hash_max);
}

static uint32_t 
hashNext(struct AStar *a, uint32_t hash) {
    return MODULO_POWER_OF_TWO(hash + 1, a->hash_max);
}

static inline struct AStarNode* 
hashPut(struct AStar *a, struct AStarNode n)
{
   if(a->hash_c >= a->hash_max)
       return NULL;

    uint32_t hash = hashFn(a, n.pos);
    uint32_t start = hash;
    do{
        struct AStarNode *d = &a->hash[hash];
        if(d->state == EMPTY_HASH){
            *d = n;
            a->hash_c++;
            return d;
        } 
        hash = hashNext(a, hash);
        }while(hash != start);

    return NULL;
}

static struct AStarNode*
hashGet(struct AStar *a, vec16 pos)
{
    uint32_t hash = hashFn(a, pos);
    uint32_t start = hash;
    do{
        struct AStarNode *n = &a->hash[hash];
        if(n->state == EMPTY_HASH){
            return NULL;
        }
        if(vec16Equal(pos, n->pos)){
            return n;
        }
        hash = hashNext(a, hash);
    }while(hash != start);

    return NULL;
}

static ptrdiff_t
hashIndex(struct AStar *a, struct AStarNode *n){
    return n - a->hash;
}

static inline uint32_t 
heapParent(uint32_t heap_i)
{
    return (heap_i - 1) / 2;
}

static inline double
fScore(struct AStarNode *n){
    return n->g + n->h;
}

static inline struct AStarNode*
nodeFromHeap(struct AStar *a, uint32_t heap_i){
    return &a->hash[a->heap[heap_i]];
}

/* assumes e is in q->heap
 * sifts node towards root, so lowest f score is on top of heap
 */
static void
siftToRoot(struct AStar *a, struct AStarNode *n)
{
    uint32_t i = n->heap_i;
    while(i > 0){
        uint32_t p = heapParent(n->heap_i); 
        struct AStarNode *parent = nodeFromHeap(a, p);
        struct AStarNode *current = nodeFromHeap(a, i);

        if(fScore(parent) <= fScore(current))
            break;

        uint32_t swap = a->heap[i];
        a->heap[i] = a->heap[p];
        a->heap[p] = swap;

        current->heap_i = p;
        parent->heap_i = i;

        i = p;
    }
}

static inline uint32_t 
heapChildLeft(uint32_t i){
    return 2 * i + 1;
}

static inline uint32_t
heapChildRight(uint32_t i){
    return 2 * i + 2;
}

static void 
siftToLeaf(struct AStar *a, struct AStarNode *n)
{
    uint32_t i = n->heap_i;
    do{
        uint32_t l = heapChildLeft(i);
        uint32_t r = heapChildRight(i);
        uint32_t smallest = i;

        struct AStarNode *left = nodeFromHeap(a, l);
        struct AStarNode *right = nodeFromHeap(a, r);
        struct AStarNode *current = nodeFromHeap(a, smallest);

        if(l < a->heap_top && 
            fScore(left) < fScore(current)) {
            smallest = l;
        }

        if(r < a->heap_top &&
            fScore(right) < fScore(current)) {
            smallest = r;
        }

        if(smallest == i) break;

        struct AStarNode *small = nodeFromHeap(a, smallest);

        uint32_t swap = a->heap[i];
        a->heap[i] = a->heap[smallest];
        a->heap[smallest] = swap;

        current->heap_i = smallest;
        small->heap_i = i;

        i = smallest;
    }while(1);
}

static int
nodeInsert(struct AStar *a, struct AStarNode in)
{
    struct AStarNode *node;
    if(a->heap_top + 1 >= a->heap_max) return 1;
    node = hashPut(a, in);
    node->state = OPEN;
    a->heap[a->heap_top] = hashIndex(a, node);
    a->heap_top++;
    siftToRoot(a, node);

    return 0;
}

static void
nodeUpdate(struct AStar *a, struct AStarNode in, struct AStarNode *dest)
{
        double old_pri = fScore(dest);
        *dest = in;
        double pri = fScore(dest);

        if(pri < old_pri)
            siftToRoot(a, dest);
        else if ( pri > old_pri)
            siftToLeaf(a, dest);
}

static int
nodePop(struct AStar *a, struct AStarNode *out)
{
    if(a->heap_top == 0) return 1;
    
    struct AStarNode *node = &a->hash[a->heap[0]];
    node->state = CLOSED;
    *out = *node;
    a->heap[0] = a->heap[a->heap_top -1 ];
    nodeFromHeap(a, 0)->heap_i = 0;
    a->heap_top--;

    if(a->heap_top > 0) {
        siftToLeaf(a, &a->hash[a->heap[0]]);
    }

    return 0;
}

int aStarBuildPath(struct AStar *a, vec16 *path, uint32_t *path_len){

    struct AStarNode *n = hashGet(a, a->goal);
    if(!n) return 1;

    if(!path){
        *path_len = 0;

        vec16 cur;
        for(vec16Copy(a->goal, cur); 
            !vec16Equal(cur, a->start); 
            vec16Copy(n->parent, cur)){
            
            n = hashGet(a, cur);
            if(!n) break;
            (*path_len)++;
        }
        return 0;
    } else {
        vec16 cur;
        uint32_t i = 0;

        for(vec16Copy(a->goal, cur);
            i < *path_len && !vec16Equal(cur, a->start);
            vec16Copy(n->parent, cur)){

            n = hashGet(a, cur);
            if(!n) break;
            vec16Copy(n->pos, path[i]);
            i++;
        }
        *path_len = i;
        free(a->heap);
        free(a->hash);
        free(a);
        return 0;
    }

    return 1;
}

static void
getNesw_(struct Space *s, vec16 tmp,
        vec16 nesw[4], uint32_t *c)
{
    struct TerraPos p;
    p = terraPos(s, tmp, 0);
    if(!terraGetSolid(p)){
        vec16Copy(tmp, nesw[*c]); 
        (*c)++;
    }
}

static void getNesw(struct Space *s, vec16 o, 
        vec16 nesw[4], uint32_t *c)
{  
    vec16 tmp;
    vec16Copy(VEC16_NORTH(o), tmp);   // E
    getNesw_(s, tmp, nesw, c);
    vec16Copy(VEC16_EAST(o), tmp); 
    getNesw_(s, tmp, nesw, c);
    vec16Copy(VEC16_SOUTH(o), tmp);
    getNesw_(s, tmp, nesw, c);
    vec16Copy(VEC16_WEST(o), tmp);
    getNesw_(s, tmp, nesw, c);
}

int aStar(struct Dungeon *d, vec16 start, vec16 goal, struct AStar **aa){

    struct AStar *a = pQueCreate();
    vec16Copy(start, a->start);
    vec16Copy(goal, a->goal);

    struct AStarNode first = {0};
    first.h = manhattanDist(start, goal),
    vec16Copy(start, first.pos);
    vec16Copy(start, first.parent);
    nodeInsert(a, first);

    int debug_c = 0;

    struct AStarNode current;
    while(nodePop(a, &current) == 0){

        debug_c++;

        if(vec16Equal(current.pos, goal))
            goto end;            
    
        vec16 nesw[4]; uint32_t nesw_c = 0;
        getNesw(d->space, current.pos, nesw, &nesw_c);

        for(uint32_t i = 0; i < nesw_c; i++){

            struct AStarNode *neighbor = hashGet(a, nesw[i]);

            if(!neighbor){
                struct AStarNode insert = {0};
                insert.g = current.g + manhattanDist(current.pos, nesw[i]);
                insert.h = manhattanDist(nesw[i], goal);
                vec16Copy(nesw[i], insert.pos);
                vec16Copy(current.pos, insert.parent);
                if(nodeInsert(a, insert) > 0)
                    goto end; // out of memory;
                continue;
            }

            if(neighbor->state == OPEN){
                double g = current.g + manhattanDist(current.pos, neighbor->pos);
                if(g < neighbor->g){
                    struct AStarNode update = *neighbor;
                    update.g = g;
                    vec16Copy(current.pos, update.parent);
                    nodeUpdate(a, update, neighbor); 
                }
            }

        }
    }

end:
    //free(a->heap);
    //free(a->hash);
    //free(a);

    *aa = a;

    return 0;
}
