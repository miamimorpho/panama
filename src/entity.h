#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include "ivec16.h"
#include "handle.h"
#include "vector.h"
#include "terminal_types.h"

VECTOR(Inventory, Handle);

struct Archetype {

	// Memory Management
	uint32_t max;
	uint32_t cur;

	// Components Begin
	char **name;
	utf8_ch *glyph;
	enum ColorSym *color;

	// Location
	struct Space *space;
	struct Inventory *inventory; // if entity stores items, this is an array of
								 // the Handles of the entities contained.
	Handle *inventory_host; // if entity is stored within another, this points
							// to its container entity

	// Actor Stats
	int32_t *hp;

	uint8_t alive;

	// Body
	uint32_t *str;
	uint32_t *con;
	uint32_t *dex;
	uint32_t *per;
	uint32_t *wis;
	uint32_t *cha;

	// Weapons Stats
	uint32_t *attack;
	uint32_t *range;
	uint32_t *damage;
};
typedef struct Archetype Entities[ARCHETYPE_MAX_C];

struct Dungeon {
	struct Terrain *terrain;
	Entities entt;
};

void dungeonCreate(struct Dungeon *);
int dungeonGenerate(struct Dungeon *, const char *);

int entityJson(Entities, const char *filename, Handle *);

int entityIsDead(Entities, Handle);
Handle entityGet(Entities, ArchetypeEnum, vec16);
int entityWhere(Entities, Handle, vec16);
void entityMove(struct Dungeon *, Handle, vec16);
int entityAttack(Entities, Handle, Handle);
int entityPickUp(Entities all_types, Handle grabber, Handle item);

#endif // ENTITY_H
