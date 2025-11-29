#include "entity.h"
#include "terra.h"
#include "space.h"

static const int MONSTER_MAX_C = 128;
static const int MONSTER_GRANULARITY = 16;
static const int ITEM_MAX_C = 128;
static const int ITEM_GRANULARITY = 16;

int
monstersCreate(struct Archetype *a)
{
	a->max = MONSTER_MAX_C;
	a->cur = 0;
	a->space = spaceCreate(MONSTER_GRANULARITY, MONSTER_MAX_C);

	COMPONENT_ADD(a, names, MONSTER_MAX_C);
	COMPONENT_ADD(a, tiles, MONSTER_MAX_C);
	COMPONENT_ADD(a, inventory, MONSTER_MAX_C);
	for (int i = 0; i < a->max; i++) {
		VECTOR_CREATE(&a->inventory[i], 24);
	}
	COMPONENT_ADD(a, hp, MONSTER_MAX_C);
	COMPONENT_ADD(a, str, MONSTER_MAX_C);
	COMPONENT_ADD(a, con, MONSTER_MAX_C);
	COMPONENT_ADD(a, per, MONSTER_MAX_C);
	COMPONENT_ADD(a, dex, MONSTER_MAX_C);
	COMPONENT_ADD(a, wis, MONSTER_MAX_C);
	COMPONENT_ADD(a, cha, MONSTER_MAX_C);

	return 0;
}

int
itemsCreate(struct Archetype *a)
{
	a->max = ITEM_MAX_C;
	a->cur = 0;
	a->space = spaceCreate(ITEM_GRANULARITY, ITEM_MAX_C);

	COMPONENT_ADD(a, names, ITEM_MAX_C);
	COMPONENT_ADD(a, tiles, ITEM_MAX_C);
	COMPONENT_ADD(a, inventory_host, ITEM_MAX_C);
	COMPONENT_ADD(a, hp, ITEM_MAX_C);
	COMPONENT_ADD(a, bonus, ITEM_MAX_C);
	COMPONENT_ADD(a, def_type, ITEM_MAX_C);
	COMPONENT_ADD(a, range, ITEM_MAX_C);
	COMPONENT_ADD(a, damage, ITEM_MAX_C);

	return 0;
}

void
dungeonCreate(struct Dungeon *d)
{
	d->terrain = terrainCreate();
	monstersCreate(&d->entt[ARCHETYPE_MONSTER]);
	itemsCreate(&d->entt[ARCHETYPE_ITEM]);
}
