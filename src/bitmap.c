#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bitmap.h"

struct Bitmap {
	uint32_t width;
	uint32_t height;
	char data[];
};

// Calculate data size in bytes for given dimensions
static size_t
bitmapDataSize(uint32_t width, uint32_t height)
{
	return ((uint64_t) width * height + 7) / 8; // Prevent overflow, round up
}

struct Bitmap *
bitmapCreate(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0)
		return NULL;

	size_t data_size = bitmapDataSize(width, height);
	size_t total_size = sizeof(struct Bitmap) + data_size;

	struct Bitmap *bmp = malloc(total_size);
	if (!bmp)
		return NULL;

	bmp->width = width;
	bmp->height = height;
	memset(bmp->data, 0, data_size);

	return bmp;
}

bool
bitmapGetPx(struct Bitmap *bmp, int32_t x, int32_t y, bool oob)
{
	if (!bmp)
		return oob;
	if (x < 0 || x > (int32_t) bmp->width || y < 0 || y > (int32_t) bmp->height)
		return oob;

	uint32_t bit_index = y * bmp->width + x;
	uint32_t byte_index = bit_index / 8;
	uint32_t bit_offset = bit_index % 8;

	return BITFIELD_EXTRACT(bmp->data[byte_index], bit_offset, 1);
}

void
bitmapPutPx(struct Bitmap *bmp, int32_t x, int32_t y, bool val)
{
	if (!bmp)
		return;
	if (x < 0 || x > (int32_t) bmp->width || y < 0 || y > (int32_t) bmp->height)
		return;

	uint32_t bit_index = y * bmp->width + x;
	uint32_t byte_index = bit_index / 8;
	uint32_t bit_offset = bit_index % 8;

	bmp->data[byte_index] =
		BITFIELD_INSERT(bmp->data[byte_index], val ? 1 : 0, bit_offset, 1);
}

// Fill entire bitmap with value (0 or 1)
void
bitmapFill(struct Bitmap *bmp, bool val)
{
	if (!bmp)
		return;

	size_t data_size = bitmapDataSize(bmp->width, bmp->height);
	memset(bmp->data, val ? 0xFF : 0x00, data_size);
}
