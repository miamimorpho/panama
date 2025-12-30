#ifndef TERMINAL_TYPES_H
#define TERMINAL_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include "utf8.h"

enum ColorSym {
	COLOR_BG,
	COLOR_FG,
	COLOR_MONSTER,
	COLOR_ITEM,
	COLOR_COUNT,
};

extern const char *colorname[COLOR_COUNT];

enum ColorMode {
	COLOR_MODE_MONO,
	COLOR_MODE_16,
	COLOR_MODE_256,
	COLOR_MODE_TRUE
};

typedef union {
	bool mono;		 // 0 = default, 1 = inverted
	uint8_t ansi16;	 // 0–15 index in the 16‑colour set
	uint8_t ansi256; // 0–255 index in xterm 256‑colour space
	struct {
		uint8_t r, g, b; // truecolor 0–255 each
	} rgb;
} Color;

typedef Color ColorPalette[COLOR_COUNT];

struct TermTile {
	utf8_ch utf;
	Color fg;
	Color bg;
};

#endif
