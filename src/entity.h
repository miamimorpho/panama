#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include "ivec16.h"
#include "dungeon.h"
#include "handle.h"
#include "vector.h"

VECTOR(Inventory, Handle);

struct Archetype {

	// Memory Management
	uint32_t max;
	uint32_t cur;

	char **names;
	struct TermTile *tiles;

	// Physics
	struct Space *space;
	struct Inventory *inventory;
	Handle *inventory_host;

	// Stats
	int32_t *hp;
	uint32_t *atk;
	uint32_t *def;
};
typedef struct Archetype Entities[ARCHETYPE_MAX_C];

struct Dungeon {
	struct Terrain *terrain;
	Entities entt;
};

void *componentMalloc(size_t, size_t);
#define COMPONENT_ADD(arch, field, count)                                      \
	arch->field = componentMalloc(count, sizeof(arch->field[0]))

int entityCreate(Entities all_types, ArchetypeEnum, vec16, Handle *);
int entityJson(Entities, const char *filename, Handle);
int entityIsDead(Entities, Handle);
Handle entityGet(Entities, ArchetypeEnum, vec16);
int entityWhere(Entities, Handle, vec16);
void entityMove(struct Dungeon *, Handle, vec16);
int entityAttack(Entities, Handle, Handle);
int entityPickUp(Entities all_types, Handle grabber, Handle item);

#endif // ENTITY_H
