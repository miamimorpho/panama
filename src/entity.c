#include <assert.h>
#include <string.h>

#include "entity.h"
#include "space.h"
#include "json.h"

static inline size_t
componentOverflow(size_t nmemb, size_t stride)
{
	if (stride == 0 || nmemb == 0)
		return 0;

	assert(nmemb < SIZE_MAX / stride);

	return nmemb * stride;
}

void *
componentMalloc(size_t nmemb, size_t stride)
{
	size_t sz;
	if (!(sz = componentOverflow(nmemb, stride)))
		return NULL;

	void *ptr;
	if (!(ptr = malloc(sz)))
		return NULL;

	memset(ptr, 0, sz);
	return ptr;
}

int
entityCreate(Entities all_types, ArchetypeEnum type, vec16 where, Handle *out)
{
	struct Archetype *a = &all_types[type];
	if (a->cur >= a->max)
		return 1;
	out->type = type;
	out->id = a->cur++;
	spaceInsert(a->space, out->id, where);
	return 0;
}

int
entityJson(Entities all_types, const char *filename, Handle han)
{
	cJSON *json = readJson(filename);
	if (!json)
		return 1;

	struct Archetype *a = &all_types[han.type];

	readJsonCopyString(json, "name", a->names, han.id);
	readJsonCopyChar(json, "tile", a->tiles, han.id);
	readJsonCopyUint32(json, "atk", a->atk, han.id);
	readJsonCopyUint32(json, "hp", a->hp, han.id);
	readJsonCopyUint32(json, "def", a->def, han.id);

	cJSON_Delete(json);
	return 0;
}

int
entityIsDead(Entities all_types, Handle in)
{
	struct Archetype *type = &all_types[in.type];
	if (!type->hp)
		return 1;
	return (type->hp[in.id] < 0);
}

int
entityGet(Entities all_types, ArchetypeEnum type, vec16 in, HandleID *out)
{
	struct Archetype *a = &all_types[type];
	if (!a->space)
		return 1;
	return spaceGet(a->space, out, in);
}

int
entityWhere(Entities all_types, Handle in, vec16 out)
{
	struct Archetype *type = &all_types[in.type];
	if (!type->space)
		return 1;
	return spaceWhere(type->space, in.id, out);
}

void
entityMove(Entities all_types, Handle in, vec16 delta)
{
	struct Archetype *type = &all_types[in.type];
	if (!type->space)
		return;
	spaceMove(type->space, in.id, delta);
}

int
entityAttack(Entities all_types, Handle atk, Handle def)
{
	struct Archetype *atk_type = &all_types[atk.type];
	struct Archetype *def_type = &all_types[def.type];

	def_type->hp[def.id] -= atk_type->atk[atk.id];

	if (def_type->hp[def.id] <= 0) {
		def_type->tiles[def.id].utf = utf8Char("X");
	}

	return 0;
}

/*
HandleID
entityRangeStart(Entities all_types, ArchetypeEnum type,
		struct SpaceFinder* f, vec16 o, uint32_t r){
	struct Archetype *a = &all_types[type];
	return spaceFindStart(a->space, f, o, r);
}

HandleID
entityRangeNext(Entities all_types, ArchetypeEnum type,
HandleID spaceFindNext(struct SpaceFinder *f, HandleID);
*/
