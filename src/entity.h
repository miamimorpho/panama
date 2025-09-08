#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include "ivec16.h"
#include "terminal_types.h"

typedef uint64_t HandleID;
static const HandleID NULL_HANDLE = UINT64_MAX;

typedef enum {
	ARCHETYPE_NONE = 0,
	ARCHETYPE_MONSTER,
	ARCHETYPE_ITEM,
	ARCHETYPE_MAX_C,
} ArchetypeEnum;

typedef struct {
	ArchetypeEnum type;
	HandleID id;
} Handle;

typedef int (*ObjCreateFn)(void *, HandleID *);
typedef int (*ObjJsonFn)(void *, HandleID *, const char *);
/* turns a struct { archetype, id } to a unique handleID for that type
 */
typedef int (*ObjDerefFn)(void *, HandleID *, Handle);

/* turns a position into a unique handleID
 */
typedef int (*ObjGetFn)(void *, HandleID *, vec16);
/* inverse of get, turns a unique handle to a position
 */
typedef int (*ObjWhereFn)(void *, HandleID *, vec16);
typedef int (*ObjMoveFn)(void *, HandleID *, vec16);
typedef int (*ObjTileFn)(void *, HandleID *, char **);
typedef int (*ObjIsOpaque)(void *, HandleID *);
typedef int (*ObjIsSolid)(void *, HandleID *);
typedef int (*ObjMoveInto)(void *, HandleID *, Handle);

struct VTable {
	ObjDerefFn deref;
	ObjGetFn get;
	ObjCreateFn create;
	ObjJsonFn json;
	ObjWhereFn where;
	ObjTileFn tile;
};

struct Archetype {
	ArchetypeEnum type;
	struct VTable *vtable_;

	// Memory Management
	uint32_t max;
	uint32_t cur;
	char **names;
	struct TermTile *tiles;

	// Physics
	struct Space *space;
	Handle **container_arr;
	Handle *container_host;

	// Stats
	uint32_t *hp;
	uint32_t *atk;
	uint32_t *def;
};
typedef struct Archetype *Entities;

void *componentMalloc(size_t, size_t);
#define COMPONENT_ADD(arch, field, count)                                      \
	arch->field = componentMalloc(count, sizeof(arch->field[0]))

int entityCreate(Entities all_types, ArchetypeEnum, vec16, Handle *);
int entityJson(Entities, const char *filename, Handle);
int entityIsDead(Entities, Handle);
int entityGet(Entities, ArchetypeEnum, vec16, HandleID *);
int entityWhere(Entities, Handle, vec16);
void entityMove(Entities, Handle, vec16);

#endif // ENTITY_H
