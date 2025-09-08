#include "dungeon.h"

#include "terra.h"
#include "monster.h"
#include "items.h"

void
dungeonCreate(struct Dungeon *d)
{
	d->terrain = terrainCreate();
	monstersCreate(&d->entt[ARCHETYPE_MONSTER]);
	itemsCreate(&d->entt[ARCHETYPE_ITEM]);
}
