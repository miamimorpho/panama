#include "monster.h"
#include "entity.h"
#include "space.h"

#define MONSTER_MAX_C 256
#define LOOKUP_GRANULARITY 16

int
monstersCreate(struct Archetype *a)
{
	a->max = MONSTER_MAX_C;
	a->cur = 0;

	a->space = spaceCreate(LOOKUP_GRANULARITY, MONSTER_MAX_C);

	COMPONENT_ADD(a, names, MONSTER_MAX_C);
	COMPONENT_ADD(a, tiles, MONSTER_MAX_C);
	COMPONENT_ADD(a, container_host, MONSTER_MAX_C);
	COMPONENT_ADD(a, container_arr, MONSTER_MAX_C);
	COMPONENT_ADD(a, atk, MONSTER_MAX_C);
	COMPONENT_ADD(a, def, MONSTER_MAX_C);
	COMPONENT_ADD(a, hp, MONSTER_MAX_C);

	return 0;
}

/*
int
monsterPickupItem(struct Dungeon *d, MonsterID in)
{
	struct Monsters *m = d->monsters;
	Handle *h = m->inventory[in.id];
	struct Items *items = d->items;

	ItemID item;
	vec16 target; monsterWhere(m, in, target);
	if (spaceGet(items->lookup, target, &item.id))
		return 1;

	//mobileMoveInto(d->space, &d->items->mobs, item, mob, h);
	Handle monster = { MONSTERS, in.id };
	d->vtables[ITEMS].move_inventory(d, monster, monster);

	return 0;
}
*/
