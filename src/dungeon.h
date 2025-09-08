#ifndef DUNGEON_H
#define DUNGEON_H

#include "ivec16.h"
#include "entity.h"

// TODO vtable need to correspond to meaningful in game actions
// not engine specific
// moving -> anything
//
// hitting -> actor / furniture
//
// picking up -> actor / any items
// drinking -> actor / any item

// host     pickup
// target   move_inventory

struct Dungeon;

struct Dungeon {
	struct Terrain *terrain;
	struct Archetype entt[ARCHETYPE_MAX_C];
};

void dungeonCreate(struct Dungeon *);
void dungeonGenerate(struct Dungeon *);

#endif // DUNGEON_H
