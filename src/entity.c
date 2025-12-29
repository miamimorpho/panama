#include <assert.h>
#include <string.h>

#include "terra.h"
#include "entity.h"
#include "space.h"
#include "json_wrapper.h"

static const int ENTT_CHUNK_SIZE = 16;

#define COMPONENT_QUERY_ADD(comp_name, arch, count, comps_json)			\
  do {																	\
	if (jsonArrayHasEntry(comps_json, #comp_name)) {					\
	  arch->comp_name =													\
		componentMalloc(count, sizeof(arch->comp_name[0]));				\
	}																	\
  } while (0)

static inline size_t
componentOverflow(size_t nmemb, size_t stride)
{
	if (stride == 0 || nmemb == 0)
		return 0;

	assert(nmemb < SIZE_MAX / stride);

	return nmemb * stride;
}

static inline void *
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


static int
archetypeCreateOne(struct Archetype *form, json_value *entry)
{
	*form = (struct Archetype){0};
	
	int c = *jsonGetInt(entry, "count");
	form->max = c;
	form->cur = 0;
	form->space = spaceCreate(ENTT_CHUNK_SIZE, c);

	json_value *comps = jsonFindField(entry, "components");
	
	COMPONENT_QUERY_ADD(name, form, c, comps);
	COMPONENT_QUERY_ADD(glyph, form, c, comps);
	COMPONENT_QUERY_ADD(color, form, c, comps);	
	COMPONENT_QUERY_ADD(hp, form, c, comps);
	COMPONENT_QUERY_ADD(str, form, c, comps);
	COMPONENT_QUERY_ADD(con, form, c, comps);
	COMPONENT_QUERY_ADD(per, form, c, comps);
	COMPONENT_QUERY_ADD(dex, form, c, comps);
	COMPONENT_QUERY_ADD(wis, form, c, comps);
	COMPONENT_QUERY_ADD(cha, form, c, comps);
	COMPONENT_QUERY_ADD(name, form, c, comps);
	COMPONENT_QUERY_ADD(inventory, form, c, comps);
	COMPONENT_QUERY_ADD(inventory_host, form, c, comps);
	COMPONENT_QUERY_ADD(attack, form, c, comps);
	COMPONENT_QUERY_ADD(range, form, c, comps);
	COMPONENT_QUERY_ADD(damage, form, c, comps);

	return 0;
}

static int
archetypesCreate(struct Dungeon *d)
{
	json_value *root = jsonReadFile("archetypes");

	json_value *jactor = jsonFindField(root, "actor");
	archetypeCreateOne(&d->entt[ARCHETYPE_MONSTER], jactor);

	json_value *jitem = jsonFindField(root, "item");
	archetypeCreateOne(&d->entt[ARCHETYPE_ITEM], jitem);	

	json_value_free(root);
	
	return 0;
}

void
dungeonCreate(struct Dungeon *d)
{
	d->terrain = terrainCreate();
	archetypesCreate(d);
}

int
entityAlloc(struct Archetype *a, HandleID *out)
{
	if (a->cur >= a->max) {
		return 1;
	}
	*out = a->cur++;
	spaceInsert(a->space, *out, (vec16) {0, 0});

	return 0;
}

int
readJsonCopyChar(json_value *root, const char *key, utf8_ch *array,
				 size_t index)
{
	if (!array)
		return 0; // Skip if array is NULL
	if (!root || !key)
		return 1;
	const char * str = jsonGetString(root, key);

	array[index] = utf8Decomp(str);
	return 0;
}

int
readJsonCopyInt(json_value *root, const char *key, int *array, size_t index)
{
	if (!array) return 0;
	
	array[index] = *jsonGetInt(root, key);
	return 0;
}

int
readJsonCopyString(json_value *root, const char *key, char **array, size_t index)
{
	if (!array) return 0;
	const char* str = jsonGetString(root, key);

	size_t len = strlen(str) + 1;
	assert(!array[index]);
	array[index] = calloc(len, sizeof(char));
	assert(array[index] && "memory allocation failed");
	
	memcpy(array[index], str, len);
	return 0;	
}

int
entityJson(Entities all_types, const char *filename, Handle *out)
{
	HandleID id;

	json_value *r = jsonReadFile(filename);
	if (!r)
		return 1;

	struct Archetype *a = NULL;

	const char *jarchetype = jsonGetString(r, "archetype");
	size_t jlen = strlen(jarchetype);
	if (jarchetype)
	{
		if (strncmp(jarchetype, "actor", jlen) == 0) {
			out->type = ARCHETYPE_MONSTER;
		} else if (strncmp(jarchetype, "item", jlen) == 0){
			out->type = ARCHETYPE_MONSTER;
		} else {
			return 1;
		}
		a = &all_types[out->type];
	} else {
		return 1;
	}	

	entityAlloc(a, &id);
	out->id = id;

	// MetaData
	readJsonCopyString(r, "name", a->name, id);
	readJsonCopyChar(r, "glyph", a->glyph, id);
	
	readJsonCopyInt(r, "hp", a->hp, id);

	// Actor Stats
	readJsonCopyInt(r, "str", a->str, id);
	readJsonCopyInt(r, "con", a->con, id);
	readJsonCopyInt(r, "per", a->per, id);
	readJsonCopyInt(r, "dex", a->dex, id);
	readJsonCopyInt(r, "wis", a->wis, id);
	
	// Weapon Stats
	readJsonCopyInt(r, "attack", a->attack, id);
	readJsonCopyInt(r, "range", a->range, id);
	readJsonCopyInt(r, "damage", a->damage, id);

	int inventory_c = *jsonGetInt(r, "inventory");
	VECTOR_CREATE(&a->inventory[id], inventory_c);

	json_value_free(r);
	return 0;
}

int
entityIsDead(Entities all_types, Handle in)
{
	struct Archetype *type = &all_types[in.type];
	if (!type->hp)
		return 1;
	return (type->hp[in.id] <= 0);
}

Handle
entityGet(Entities all_types, ArchetypeEnum type, vec16 in)
{
	Handle out = {0};
	struct Archetype *a = &all_types[type];
	if (!a->space)
		return out;
	HandleID id;
	if (spaceGet(a->space, &id, in)) {
		return out;
	}
	return (Handle) {type, id};
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
entityMove(struct Dungeon *d, Handle in, vec16 delta)
{
	struct Archetype *type = &d->entt[in.type];
	if (!type->space)
		return;

	struct TerraPos tp = terraPos(d->terrain, delta);
	if (terraGetSolid(tp))
		return;

	spaceMove(type->space, in.id, delta);
}

int
entityPickUp(Entities all_types, Handle grabber, Handle item)
{
	struct Archetype *gra_t = &all_types[grabber.type];
	struct Archetype *itm_t = &all_types[item.type];

	if (!itm_t->inventory_host)
		return 1;
	if (!gra_t->inventory || VECTOR_FULL(gra_t->inventory))
		return 1;
	if (!item.type)
		return 1;

	spaceRemove(itm_t->space, item.id);
	itm_t->inventory_host[item.id] = grabber;
	VECTOR_PUSH(&gra_t->inventory[grabber.id], item);

	return 0;
}

int
entityAttack(Entities entts, Handle atk, Handle def)
{

	struct Archetype *atk_entt = &entts[atk.type];
	struct Archetype *def_entt = &entts[def.type];

	int atk_score = atk_entt->str[atk.id];
	def_entt->hp[def.id] -= atk_score;

	if (def_entt->hp[def.id] <= 0) {

		def_entt->glyph[def.id] = utf8Decomp("X");
	}

	return 0;
}
