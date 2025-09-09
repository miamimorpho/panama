#include <stdio.h>
#include <stdlib.h>

// Core vector struct - same layout for all types
#define VECTOR(name, T)                                                        \
	struct name {                                                              \
		T *data;                                                               \
		size_t len, cap;                                                       \
	}

// Unsafe macros - no bounds checking, caller responsibility
#define VECTOR_CREATE(v, initial_cap)                                          \
	do {                                                                       \
		(v)->data = calloc((initial_cap), sizeof(*(v)->data));                 \
		(v)->len = 0;                                                          \
		(v)->cap = (initial_cap);                                              \
	} while (0)

#define vec_free(v) free((v)->data)

#define VECTOR_PUSH(v, item)                                                   \
	do {                                                                       \
		assert((v)->len < (v)->cap && "Vector too full");                      \
		(v)->data[(v)->len++] = (item);                                        \
	} while (0)

#define VECTOR_GROW(v)                                                         \
	do {                                                                       \
		(v)->cap = (v)->cap ? (v)->cap * 2 : 1;                                \
		(v)->data = realloc((v)->data, sizeof(*(v)->data) * (v)->cap);         \
	} while (0)

#define VECTOR_SWAP_POP(v, i) ((v)->data[i] = (v)->data[--(v)->len])

#define VECTOR_EACH(v, iter)                                                   \
	for (iter = &(v)->data[0]; iter != (v)->data + (v)->len; iter++)\
