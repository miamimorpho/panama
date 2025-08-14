#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

typedef int16_t vec16[2];

#define VEC16_NORTH(o) ((vec16){ o[0]    , o[1] - 1})
#define VEC16_EAST(o)  ((vec16){ o[0] +1 , o[1]    })
#define VEC16_SOUTH(o) ((vec16){ o[0]    , o[1] + 1})
#define VEC16_WEST(o)  ((vec16){ o[0] -1 , o[1]    })

static inline int32_t
manhattanDist(vec16 a, vec16 b){
    return abs(a[0] - b[0]) + abs(a[1] - b[1]);
}

static inline void 
vec16Add(vec16 a, vec16 b, vec16 out){
    out[0] = a[0] + b[0];
    out[1] = a[1] + b[1];
}

static inline void
vec16New(int *__restrict in, vec16 out){
    out[0] = in[0];
    out[1] = in[1];
}

static inline void
vec16Copy(vec16 in, vec16 out){
    out[0] = in[0];
    out[1] = in[1];
}

static inline bool 
vec16Equal(vec16 a, vec16 b){
    return (a[0] == b[0] && a[1] == b[1]);
}

static inline void
vec16Handle(vec16 v, uint32_t *out){
    *out = ((uint32_t)v[1] << 16) | (uint32_t)v[0];
}

static inline void
vec16Comp(uint32_t h, vec16 out){
    out[0] = (int16_t)(h & 0xFFFF); 
    out[1] = (int16_t)((h >> 16) & 0xFFFF);
}

