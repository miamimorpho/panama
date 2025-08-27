#ifndef MATHS_H
#define MATHS_H

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

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

/**
 * Floor Division - Always rounds toward negative infinity
 * 
 * Unlike C's built-in division which truncates toward zero:
 *   C division:    -1 / 16 = 0,  -17 / 16 = -1
 *   Floor division: -1 / 16 = -1, -17 / 16 = -2
 * 
 * This is essential for consistent coordinate-to-chunk mapping.
 * Step 1: Start with C's truncating division
 *  int16_t truncated_result = a / b;
 *   
 * Step 2: Check if we have a remainder (division wasn't exact)
 *  bool has_remainder = (a % b != 0);
 *   
 * Step 3: Check if signs are different (XOR = exclusive OR)
 *  This tells us if we're dividing a negative by positive or vice versa
 *  bool signs_differ = ((a < 0) ^ (b < 0));
 *   
 * Step 4: Only adjust if we have a remainder AND signs differ
 *  When signs differ and there's a remainder, C's truncation went
 *  toward zero but we want to go toward negative infinity
 *  bool need_adjustment = has_remainder && signs_differ;
 *   
 * Step 5: Subtract 1 to "floor" the result (round toward -infinity)
 *  return truncated_result - (need_adjustment ? 1 : 0);
 *   
 *   The original compact version does all this in one line:
 */
static inline int32_t floorDiv(int32_t a, int32_t b) {
    return (a / b) - (a % b != 0 && ((a < 0) ^ (b < 0)));
}

/**
 * Floor Modulo - Always returns positive remainder in range [0, b-1]
 * 
 * Unlike C's built-in modulo which can return negative values:
 *   C modulo:     -1 % 16 = -1,  -17 % 16 = -1
 *   Floor modulo: -1 % 16 = 15,  -17 % 16 = 15
 * 
 * This ensures local chunk coordinates are always positive and valid array
 * indices.
 * 
 * Step 1: Get C's regular modulo result (might be negative)
 * int16_t c_modulo = a % b;
 *   
 * Step 2: Add the divisor to handle negative results
 * int16_t shifted = c_modulo + b;
 *   
 * Step 3: Take modulo again to wrap positive results back to [0, b-1]
 * int16_t final_result = shifted % b;
 *   
 * return final_result;
 *   
 */
static inline int32_t floorMod(int32_t a, int32_t b) {
    return ((a % b) + b) % b;
}

#endif
