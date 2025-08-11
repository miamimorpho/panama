#include <stdint.h>

typedef uint32_t tinyPtr;
#define PLIST_NULL 0

#define PLIST_HEAD(name) struct name { tinyPtr slh_first; }
#define PLIST_ENTRY() struct { tinyPtr sle_next; }

#define PLIST_FIRST(pool, head) \
    ((head)->slh_first ? &(pool)[(head)->slh_first - 1] : NULL)

#define PLIST_NEXT(pool, elm, field) \
    ((elm)->field.sle_next ? &(pool)[(elm)->field.sle_next - 1] : NULL)

#define PLIST_PTR(pool, elm) (((elm) - (pool)) + 1)

#define PLIST_EMPTY(head) (!(head)->slh_first)
#define PLIST_INIT(head) ((head)->slh_first = PLIST_NULL)

#define	PLIST_FOREACH(pool, head, tmp, field)		\
	for((tmp) = PLIST_FIRST(pool, head);			\
	    (tmp) != PLIST_NULL;				        \
	    (tmp) = PLIST_NEXT(pool, tmp, field))

#define PLIST_INSERT_HEAD(pool, head, elm, field) do { \
    (elm)->field.sle_next = (head)->slh_first; \
    (head)->slh_first = PLIST_PTR(pool, elm); \
} while (0)

#define PLIST_REMOVE_HEAD(pool, head, field) do { \
    if ((head)->slh_first) { \
        tinyPtr old_head = (head)->slh_first; \
        (head)->slh_first = (pool)[old_head - 1].field.sle_next; \
        (pool)[old_head - 1].field.sle_next = PLIST_NULL; \
    } \
} while (0)

#define PLIST_REMOVE(pool, head, elm, field) do {                \
    tinyPtr elm_idx = PLIST_PTR(pool, elm);                           \
    if ((head)->slh_first == elm_idx) {                               \
        PLIST_REMOVE_HEAD(pool, head, field);                         \
    } else {                                                          \
        tinyPtr cur_idx = (head)->slh_first;                          \
                                                                      \
        while ((pool)[cur_idx - 1].field.sle_next != elm_idx)        \
            cur_idx = (pool)[cur_idx - 1].field.sle_next;            \
        (pool)[cur_idx - 1].field.sle_next =                         \
            (pool)[elm_idx - 1].field.sle_next;                      \
    }                                                                 \
    (elm)->field.sle_next = PLIST_NULL;                              \
} while (0)
