#include <string.h>
#include <stdlib.h>
#include "terra.h"
#include "space.h"
#include "maths.h"
#include "bitmap.h"

#define CHUNK_C 128
#define CHUNK_LENGTH 16
#define LOOKUP_GRANULARITY 1

struct TerraChunk {
	Bitmap *solid;
	Bitmap *opaque;
	struct TermTile *tile;
};

struct Terrain {
	size_t max;
	size_t cur;

	struct Space *lookup;
	struct TerraChunk *chunk;
};

struct Terrain *
terrainCreate(void)
{
	struct Terrain *tm = malloc(sizeof(struct Terrain));
	tm->max = CHUNK_C;
	tm->cur = 0;

	tm->lookup = spaceCreate(LOOKUP_GRANULARITY, CHUNK_C);
	tm->chunk = calloc(CHUNK_C, sizeof(struct TerraChunk));

	for (int i = 0; i < CHUNK_C; i++) {
		struct TerraChunk *chunk = &tm->chunk[i];
		chunk->solid = bitmapCreate(CHUNK_LENGTH, CHUNK_LENGTH);
		chunk->opaque = bitmapCreate(CHUNK_LENGTH, CHUNK_LENGTH);
		chunk->tile =
			calloc(CHUNK_LENGTH * CHUNK_LENGTH, sizeof(struct TermTile));
		for (int t = 0; t < CHUNK_LENGTH * CHUNK_LENGTH; t++) {
			chunk->tile[t].utf = utf8Char(" ");
		}
	}

	return tm;
}

void
localisePos(vec16 in, vec16 chunk_out, vec16 local_out)
{
	local_out[0] = floorMod(in[0], CHUNK_LENGTH);
	local_out[1] = floorMod(in[1], CHUNK_LENGTH);
	chunk_out[0] = in[0] - local_out[0];
	chunk_out[1] = in[1] - local_out[1];
}

struct TerraPos
terraPos(struct Terrain *tm, vec16 p)
{
	vec16 chunk_pos, local_pos;
	localisePos(p, chunk_pos, local_pos);

	struct TerraChunk *tc;
	HandleID handle;
	if (spaceGet(tm->lookup, &handle, chunk_pos) == 0) {
		tc = &tm->chunk[handle];
	} else {
		tc = NULL;
	}

	struct TerraPos ter = {.chunk = tc};
	vec16Copy(local_pos, ter.pos);
	return ter;
}

static struct TerraChunk *
chunkCreate(struct Terrain *tm, vec16 where)
{
	if (tm->cur >= tm->max)
		return NULL;
	HandleID n = tm->cur++;
	spaceInsert(tm->lookup, n, where);
	return &tm->chunk[n];
}

struct TerraPos
terraPosNew(struct Terrain *tm, vec16 p)
{
	vec16 chunk_pos, local_pos;
	localisePos(p, chunk_pos, local_pos);

	struct TerraChunk *tc;
	HandleID n;
	if (spaceGet(tm->lookup, &n, chunk_pos) == 0) {
		tc = &tm->chunk[n];
	} else {
		tc = chunkCreate(tm, chunk_pos);
		if (!tc)
			exit(EXIT_FAILURE);
	}

	struct TerraPos ter = {.chunk = tc};
	vec16Copy(local_pos, ter.pos);
	return ter;
}

void
terraPutOpaque(struct TerraPos p, bool val)
{
	if (!p.chunk)
		return;
	bitmapPutPx(p.chunk->opaque, p.pos[0], p.pos[1], val);
}

bool
terraGetOpaque(struct TerraPos p)
{
	if (!p.chunk)
		return 1;
	return bitmapGetPx(p.chunk->opaque, p.pos[0], p.pos[1], 1);
}

void
terraPutSolid(struct TerraPos p, bool val)
{
	if (!p.chunk)
		return;
	bitmapPutPx(p.chunk->solid, p.pos[0], p.pos[1], val);
}

bool
terraGetSolid(struct TerraPos p)
{
	if (!p.chunk)
		return 1;
	return bitmapGetPx(p.chunk->solid, p.pos[0], p.pos[1], 1);
}

void
terraPutTile(struct TerraPos p, struct TermTile ch)
{
	if (!p.chunk)
		return;
	p.chunk->tile[p.pos[1] * CHUNK_LENGTH + p.pos[0]] = ch;
}

struct TermTile
terraGetTile(struct TerraPos p)
{
	static struct TermTile tile = {0};

	if (!p.chunk) {
		tile.utf = utf8Char("#");
		return tile;
	}
	return p.chunk->tile[p.pos[1] * CHUNK_LENGTH + p.pos[0]];
}
