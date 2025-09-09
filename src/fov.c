#include "fov.h"
#include "entity.h"

enum Cardinal {
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3,
};

typedef struct {
	long long num;
	long long den;
} Fraction;

struct Row {
	vec16 camera;
	enum Cardinal dir;
	int depth;
	Fraction start;
	Fraction end;
};

static void
shadowToWorldPos(enum Cardinal dir, uint32_t depth, uint32_t col, vec16 cam,
				 vec16 out)
{
	switch (dir) {
	case NORTH:
		out[0] = cam[0] + col;
		out[1] = cam[1] - depth;
		break;
	case SOUTH:
		out[0] = cam[0] + col;
		out[1] = cam[1] + depth;
		break;
	case EAST:
		out[0] = cam[0] + depth;
		out[1] = cam[1] + col;
		break;
	case WEST:
		out[0] = cam[0] - depth;
		out[1] = cam[1] + col;
		break;
	}
}

static inline Fraction
frScalar(Fraction a, int32_t s)
{
	return (Fraction) {
		a.num * s,
		a.den,
	};
}

static inline double
decimate(Fraction fr)
{
	return (double) fr.num / (double) fr.den;
}

static inline int32_t
frRoundTiesUp(Fraction fr)
{
	return floor(decimate(fr) + 0.5);
}

static inline int32_t
frRoundTiesDown(Fraction fr)
{
	return ceil(decimate(fr) - 0.5);
}

// fast, approximate
static inline int
frCompare(Fraction a, Fraction b)
{
	return a.num * b.den - b.num * a.den;
}

static Fraction
slope(int32_t depth, int32_t col)
{
	if (!depth)
		return (Fraction) {0, 1};

	return (Fraction) {2 * col - 1, 2 * depth};
}

static bool
isSymmetric(struct Row *row, int32_t col)
{
	return (col >= decimate(frScalar(row->start, row->depth)) &&
			col <= decimate(frScalar(row->end, row->depth)));
}

static void
scanRow(struct Row cur, struct Dungeon *d, float depth_max,
		struct FovEffect *effect)
{

	if (cur.depth > depth_max)
		return;

	if (frCompare(cur.end, cur.start) <= 0)
		return;

	int min = frRoundTiesUp(frScalar(cur.start, cur.depth));
	int max = frRoundTiesDown(frScalar(cur.end, cur.depth));

	bool prev_was_wall = false;
	for (int col = min; col <= max; col++) {
		vec16 pos;
		shadowToWorldPos(cur.dir, cur.depth, col, cur.camera, pos);

		struct TerraPos ter = terraPos(d->terrain, pos);
		bool is_wall = terraGetOpaque(ter);

		if (!is_wall || isSymmetric(&cur, col)) {
			effect->fn(effect, ter, pos);
		}
		if (prev_was_wall && !is_wall) {
			cur.start = slope(cur.depth, col);
		}
		if (!prev_was_wall && is_wall) {
			struct Row next = cur;
			next.depth += 1;
			next.end = slope(cur.depth, col);
			scanRow(next, d, depth_max, effect);
		}
		prev_was_wall = is_wall;
	}

	if (!prev_was_wall) {
		struct Row next = cur;
		next.depth += 1;
		scanRow(next, d, depth_max, effect);
	}
	return;
}

void
fov(struct Dungeon *d, vec16 o, struct FovEffect *effect)
{
	struct TerraPos ter = terraPos(d->terrain, o);
	effect->fn(effect, ter, o);
	struct Row first = {
		.depth = 1,
		.start = (Fraction) {-1, 1},
		.end = (Fraction) {1, 1},
	};
	vec16Copy(o, first.camera);
	for (int dir = 0; dir < 4; dir++) {
		first.dir = dir;
		scanRow(first, d, 32, effect);
	}
}
