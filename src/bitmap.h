#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define BITFIELD_EXTRACT(value, offset, bits) \
    (((unsigned)(value) >> (offset)) & ((1U << (bits)) - 1U))
#define BITFIELD_INSERT(base, insert, offset, bits) \
    (((unsigned)(base) & ~(((1U << (bits)) - 1U) << (offset))) | \
     ((((unsigned)(insert)) & ((1U << (bits)) - 1U)) << (offset)))

typedef struct Bitmap Bitmap;

struct Bitmap *bitmapCreate(uint32_t, uint32_t);
bool bitmapGetPx(struct Bitmap *, uint32_t, uint32_t, bool);
void bitmapPutPx(struct Bitmap *, uint32_t, uint32_t, bool);
void bitmapFill(struct Bitmap *, bool val);

#endif // BITMAP_H
