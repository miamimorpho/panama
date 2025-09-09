#ifndef HANDLE_H
#define HANDLE_H

#include <stdint.h>

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

#endif // HANDLE_H
