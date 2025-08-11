#ifndef MATHS_H
#define MATHS_H

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#define POSITION_FIELD struct{uint16_t x; uint16_t y;}
#define VEC_ADD(a_, b_) do{(a_)->x += (b_)->x; (a_)->y += (b_)->y;}while(0)

/*
 * https://nullprogram.com/blog/2018/07/31/
 * 32bit hash version 1
 */
static inline uint32_t
hashFunction32(uint32_t n)
{
	n ^= n >> 16;
	n *= 0x45d9f3bU;
	n ^= n >> 16;

	/* version 2
	 * n ^= n >> 16;
	 * n *= 0x7feb352dU;
	 * n ^= n >> 15;
	 * n *= 0x846ca68bU;
	 * n ^= n >> 16;
	 */

	return n;
}

static inline size_t sizeOverflowCheck(size_t nmemb, size_t stride)
{
    assert(stride > 0 && "stride cannot be zero");
    assert(nmemb <= SIZE_MAX / stride && "multiplication overflow");
    return nmemb * stride;
}

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

static inline uint32_t nextLowestPowerOfTwo(uint32_t n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    
    // Copy n and shift right until we find the MSB
    uint32_t result = 1;
    while (result <= n) {
        result <<= 1;
    }
    
    return result >> 1;
}

#define IS_POWER_OF_TWO(val_) \
    (((val_) != 0 && ((val_) & ((val_) - 1))) == 0)

#define MODULO_POWER_OF_TWO(val_, power_of_two_) \
    ((val_) &((power_of_two_) - 1))

#endif
