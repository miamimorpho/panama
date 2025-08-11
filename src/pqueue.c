#include <stdint.h>
#include "pqueue.h"
#include "plist.h"
#include "maths.h" 

struct PQueElm{
    PLIST_ENTRY() p; // hash table collision next
    double priority;
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
hashPut(struct PQue *q, struct PQueElm *pool, uint32_t i)
{
    struct PQueElm *e = &pool[i];
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
hashRem(struct PQue *q, struct PQueElm *pool, uint32_t i){
    struct PQueElm *e = &pool[i];
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

// assumes e is in q->heap
static void
siftToRoot(struct PQue *q, struct PQueElm *e)
{
    if(!e)return;

    uint32_t i = heapIndex(q, e);
    while(i > 0){
        uint32_t p = heapParent(i);
    
        if(q->heap[p].priority <= q->heap[i].priority)
            break;

        hashRem(q, q->heap, i);
        hashRem(q, q->heap, p);

        struct PQueElm swap = q->heap[i];
        q->heap[i] = q->heap[p];
        q->heap[p] = swap;

        hashPut(q, q->heap, i);
        hashPut(q, q->heap, p);
   
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

        if(left <= q->heap_top && 
            q->heap[left].priority < q->heap[smallest].priority) {
            smallest = left;
        }

        if(right <= q->heap_top &&
            q->heap[right].priority < q->heap[smallest].priority) {
            smallest = right;
        }

        if(smallest == i) break;

        hashRem(q, q->heap, i);
        hashRem(q, q->heap, smallest);
        struct PQueElm swap = q->heap[i];
        q->heap[i] = q->heap[smallest];
        q->heap[smallest] = swap;

        hashPut(q, q->heap, i);
        hashPut(q, q->heap, smallest);

        i = smallest;
    }while(1);
}

int
PQueInsert(struct PQue *q, uint32_t val, double pri)
{
    struct PQueElm *e;
    if((e = hashGet(q, q->heap, val)) != NULL){
        double old_pri = e->priority;
        e->priority = pri;

        if(pri < old_pri)
            siftToRoot(q, e);
        else if ( pri > old_pri)
            siftToLeaf(q, e);
   
        return 0;
    }

    if(q->heap_top + 1 >= q->max) return 1;
    e = &q->heap[++q->heap_top];
    e->priority = pri;
    e->value = val;

    hashPut(q, q->heap, q->heap_top);
    siftToRoot(q, e);

    return 0;
}

int PQueWasDone(struct PQue *q, uint32_t val){
    return hashGet(q, q->free, val) != NULL;
}

int
PQuePop(struct PQue *q, uint32_t *val_out, double *pri_out)
{
    if(q->heap_top == 0) return 1;

    *val_out = q->heap[0].value;
    *pri_out = q->heap[0].priority;

    hashRem(q, q->heap, 0);

    if(q->free_top + 1 >= q->max) return 1;
    
    q->free_top++;
    q->free[q->free_top] = q->heap[0];
    q->heap[0] = q->heap[q->heap_top--];

    hashPut(q, q->free, q->free_top);

    if(q->heap_top > 0) {
        hashPut(q, q->heap, 0);
        siftToLeaf(q, q->heap);
    }

    return 0;
}

