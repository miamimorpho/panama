#include "render.h"
#include "terminal.h"
#include "fov.h"
#include "entity.h"
#include "space.h"

struct FovDrawCtx {
	vec16 offset;
};

void
fovDrawFn(struct FovEffect *p, struct TerraPos ter, vec16 in)
{
	struct FovDrawCtx *ctx = (struct FovDrawCtx *) p->ctx;
	struct TermUI ui = {.x = in[0] - ctx->offset[0],
						.y = in[1] - ctx->offset[1]};
	termPut(&ui, terraGetTile(ter).utf);
}

// fog of war
// change draw to bitmap

int
drawDungeon(struct Dungeon *d, vec16 o)
{

	uint16_t w, h;
	termSize(&w, &h);

	int32_t cam_x = o[0] - (uint32_t) (w / 2);
	int32_t cam_y = o[1] - (uint32_t) (h / 2);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			struct TerraPos pos =
				terraPos(d->terrain, (vec16) {cam_x + x, cam_y + y});
			struct TermUI ui2 = {x, y};
			termPut(&ui2, terraGetTile(pos).utf);
		}
	}
	struct FovDrawCtx ctx;
	vec16Copy((vec16) {cam_x, cam_y}, ctx.offset);

	struct FovEffect draw = {
		.fn = fovDrawFn,
		.ctx = &ctx,
	};

	fov(d, o, &draw);

	HandleID item;
	struct SpaceFinder item_finder;
	SPACE_FIND(d->entt[ARCHETYPE_ITEM].space, o, 3, item_finder, item)
	{
		vec16 pos;
		spaceWhere(d->entt[ARCHETYPE_ITEM].space, item, pos);
		struct TermUI ui = {
			pos[0] - cam_x,
			pos[1] - cam_y,

		};
		termPut(&ui, d->entt[ARCHETYPE_ITEM].tiles[item].utf);
	}

	HandleID monster;
	struct SpaceFinder mons_finder;
	SPACE_FIND(d->entt[ARCHETYPE_MONSTER].space, o, 3, mons_finder, monster)
	{
		vec16 pos;
		spaceWhere(d->entt[ARCHETYPE_MONSTER].space, monster, pos);
		struct TermUI ui = {
			pos[0] - cam_x,
			pos[1] - cam_y,

		};
		termPut(&ui, d->entt[ARCHETYPE_MONSTER].tiles[monster].utf);
	}

	return 0;
}
