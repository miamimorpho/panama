#include <stdint.h>
#include "astar.h"
#include "plist.h"
#include "maths.h" 

/*
 * maybe change to open addressed system?
 * this speeds sifting to root/tree much quicker
 struct aStarNode{
    double g;
    double h;
    uint32_t pos;
    uint32_t parent_pos;
    uint32_t heap_index;
 };

struct aStarQueue{
    uint32_t max;
    uint32_t heap_top:
    uint32_t *heap:
    uint32_t hash_c;
    struct aStarNode *hash
 */



struct PQueElm{
    PLIST_ENTRY() p; // hash table collision next
    double h;
    double g;
    uint32_t value;
    uint32_t parent; // value of parent todo;
};
PLIST_HEAD(PQueList);

struct PQue{
    uint32_t max; // all three arrays are same sized for now
    uint32_t free_top;
    uint32_t heap_top;
    struct PQueElm *free; // closed tasks
    struct PQueElm *heap; // open tasks
    struct PQueList *hash; // stores indices into either pool
};

static inline struct PQueList *
hashFn(struct PQue *q, uint32_t val)
{
    return &q->hash[MODULO_POWER_OF_TWO(hashFunction32(val), q->max)];
}

static inline void 
hashPut(struct PQue *q, struct PQueElm *pool, struct PQueElm *e)
{
    struct PQueList *l = hashFn(q, e->value);
    PLIST_INSERT_HEAD(pool, l, e, p);   
}

static struct PQueElm* 
hashGet(struct PQue *q, struct PQueElm *pool, uint32_t val )
{
    struct PQueList *l = hashFn(q, val);
    struct PQueElm *e = NULL;
    PLIST_FOREACH(pool, l, e, p){
        if(e->value == val) return e;
    }
    return NULL;
}

static inline void 
hashRem(struct PQue *q, struct PQueElm *pool, struct PQueElm *e){
    struct PQueList *l = hashFn(q, e->value); 
    PLIST_REMOVE(pool, l, e, p);
}

// unsafe, must only use on elements we know are in heap
static inline uint32_t 
heapIndex(struct PQue *q, struct PQueElm *e){
    return e - q->heap;
}

static inline uint32_t 
heapParent(uint32_t i)
{
    return (i - 1) / 2;
}

static inline double
fScore(struct PQueElm *e){
    return e->g + e->h;
}

// assumes e is in q->heap
static void
siftToRoot(struct PQue *q, struct PQueElm *e)
{
    if(!e)return;

    uint32_t i = heapIndex(q, e);
    while(i > 0){
        uint32_t p = heapParent(i);
    
        if(fScore(q->heap + p) <= fScore(q->heap + i))
            break;

        hashRem(q, q->heap, q->heap +i);
        hashRem(q, q->heap, q->heap +p);

        struct PQueElm swap = q->heap[i];
        q->heap[i] = q->heap[p];
        q->heap[p] = swap;

        hashPut(q, q->heap, q->heap +i);
        hashPut(q, q->heap, q->heap +p);
   
        i = p;
    }
}

static inline uint32_t 
heapChildLeft(uint32_t i) {
    return 2 * i + 1;
}

static inline uint32_t
heapChildRight(uint32_t i){
    return 2 * i + 2;
}

static void 
siftToLeaf(struct PQue *q, struct PQueElm *e)
{
    if(!e) return;

    uint32_t i = heapIndex(q, e);

    do{
        uint32_t left = heapChildLeft(i);
        uint32_t right = heapChildRight(i);
        uint32_t smallest = i;

        if(left < q->heap_top && 
            fScore(q->heap + left) < fScore(q->heap + smallest)) {
            smallest = left;
        }

        if(right < q->heap_top &&
            fScore(q->heap + right) < fScore(q->heap + smallest)) {
            smallest = right;
        }

        if(smallest == i) break;

        hashRem(q, q->heap, q->heap + i);
        hashRem(q, q->heap, q->heap + smallest);
        struct PQueElm swap = q->heap[i];
        q->heap[i] = q->heap[smallest];
        q->heap[smallest] = swap;

        hashPut(q, q->heap, q->heap + i);
        hashPut(q, q->heap, q->heap + smallest);

        i = smallest;
    }while(1);
}

/*
 * value, parents, g cost, heuristic cost,
 *
 */
int
PQueInsert(struct PQue *q, uint32_t val, uint32_t parent, double g, double h)
{
    struct PQueElm *e;
    if((e = hashGet(q, q->heap, val)) != NULL){
        double old_pri = fScore(e);
        e->g = g;
        e->h = h;
        e->parent = parent;
        double pri = fScore(e);

        hashPut(q, q->heap, e);
        if(pri < old_pri)
            siftToRoot(q, e);
        else if ( pri > old_pri)
            siftToLeaf(q, e);
   
        return 0;
    }

    if(q->heap_top + 1 >= q->max) return 1;
    e = &q->heap[++q->heap_top];
    e->g = g;
    e->h = h;
    e->parent = parent;
    e->value = val;

    hashPut(q, q->heap, q->heap + q->heap_top);
    siftToRoot(q, e);

    return 0;
}

int PQueWasDone(struct PQue *q, uint32_t val){
    return hashGet(q, q->free, val) != NULL;
}

int
PQuePop(struct PQue *q, struct PQueElm *out)
{
    if(q->heap_top == 0) return 1;
    if(q->free_top + 1 >= q->max) return 2;
    
    hashRem(q, q->heap, q->heap);
    *out = q->heap[0];

    q->free_top++;
    q->free[q->free_top] = q->heap[0];
    q->heap[0] = q->heap[q->heap_top--];

    hashPut(q, q->free, q->free + q->free_top);

    if(q->heap_top > 0) {
        hashPut(q, q->heap, q->heap);
        siftToLeaf(q, q->heap);
    }

    return 0;
}

void aStarBuildPath(struct PQue *q){

    // find goal in closed pool
    // follow parents to start
    // return array

}

// astar malloc
// come up with smart malloc strategy, can cut pathfnding short 
// if memory is constrained

void aStar(void){

    // 1. Initialize queue, add start node to heap
    // 2. While heap not empty:
    //    - PQuePop() to get lowest f-score node
    //    - If it's the goal, reconstruct path using parent pointers
    //    - Get neighbors using your map functions
    //    NESW
    //    - For each neighbor:
    //      * PQueWasDone() to skip if already processed
    //      * Calculate g, h costs
    //      * PQueInsert() (handles updates automatically)
}
