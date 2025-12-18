#include "render.h"
#include "terminal.h"
#include "fov.h"
#include "entity.h"
#include "space.h"

struct FovDrawCtx {
	vec16 offset;
	Bitmap *shadow;
};

void
fovDrawFn(struct FovEffect *p, struct TerraPos ter, vec16 in)
{
	struct FovDrawCtx *ctx = (struct FovDrawCtx *) p->ctx;
	struct TermUI ui = termRoot();

	bitmapPutPx(ctx->shadow, in[0] - ctx->offset[0], in[1] - ctx->offset[1], 1);
	terraPutFog(ter, 1);
	// termMove(&ui, in[0] - ctx->offset[0], in[1] - ctx->offset[1]);
	// termPut(&ui, terraGetTile(ter).utf);
}

int
drawDungeon(struct Dungeon *d, vec16 o)
{

	uint16_t w, h;
	termSize(&w, &h);

	struct TermUI ui = termRoot();

	int32_t offset_x = o[0] - (uint32_t) (w / 2);
	int32_t offset_y = o[1] - (uint32_t) (h / 2);

	struct FovDrawCtx ctx;
	vec16Copy((vec16) {offset_x, offset_y}, ctx.offset);
	ctx.shadow = bitmapCreate(w, h);

	struct FovEffect draw = {
		.fn = fovDrawFn,
		.ctx = &ctx,
	};

	fov(d, o, &draw);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {

			termMove(&ui, x, y);
			vec16 target = {x + offset_x, y + offset_y};
			struct TerraPos ter = terraPos(d->terrain, target);

			if (bitmapGetPx(ctx.shadow, x, y, 1)) {
				termPut(&ui, terraGetTile(ter).utf);
			} else if (terraGetFog(ter) == 0) {
				termPut(&ui, UTF8_NULL);
			} else if (terraGetSolid(ter)) {
				termPut(&ui, utf8Code(0x2591));
			} else {
				termPut(&ui, UTF8_NULL);
			}
		}
	}

	HandleID item;
	struct SpaceFinder item_finder;
	SPACE_FIND(d->entt[ARCHETYPE_ITEM].space, o, 16, item_finder, item)
	{
		vec16 pos;
		spaceWhere(d->entt[ARCHETYPE_ITEM].space, item, pos);

		if (bitmapGetPx(ctx.shadow, pos[0] - offset_x, pos[1] - offset_y, 0) ==
			1) {
			termMove(&ui, pos[0] - offset_x, pos[1] - offset_y);
			termPut(&ui, d->entt[ARCHETYPE_ITEM].tile[item].utf);
		}
	}

	HandleID monster;
	struct SpaceFinder mons_finder;
	SPACE_FIND(d->entt[ARCHETYPE_MONSTER].space, o, 16, mons_finder, monster)
	{
		vec16 pos;
		spaceWhere(d->entt[ARCHETYPE_MONSTER].space, monster, pos);
		if (bitmapGetPx(ctx.shadow, pos[0] - offset_x, pos[1] - offset_y, 0) ==
			1) {
			termMove(&ui, pos[0] - offset_x, pos[1] - offset_y);
			termPut(&ui, d->entt[ARCHETYPE_MONSTER].tile[monster].utf);
		}
	}

	// termMove(&ui, o[0] - offset_x, o[1] - offset_y);
	// termPut(&ui, d-

	return 0;
}
