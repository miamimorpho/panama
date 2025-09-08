#ifndef TERMINAL_TYPES_H
#define TERMINAL_TYPES_H

#include <stdint.h>
#include "utf8.h"

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} Color;

struct TermTile {
	utf8_ch utf;
	Color fg;
	Color bg;
};

#endif
