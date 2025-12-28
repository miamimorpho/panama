#include <assert.h>
#include <string.h>

#include "terra.h"
#include "entity.h"
#include "space.h"
#include "json.h"

static const int ENTT_CHUNK_SIZE = 16;

#define COMPONENT_QUERY_ADD(comp_name, arch, count, comps_json)                \
	do {                                                                       \
		if (hasComponent(comps_json, #comp_name)) {                            \
			arch->comp_name =                                                  \
				componentMalloc(count, sizeof(arch->comp_name[0]));            \
		}                                                                      \
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
hasComponent(cJSON *comps_arr, const char *comp_name)
{
	if (!cJSON_IsArray(comps_arr))
		return 0;

	cJSON *item = NULL;
	cJSON_ArrayForEach(item, comps_arr)
	{
		if (cJSON_IsString(item) &&
			strncmp(item->valuestring, comp_name, strlen(comp_name)) == 0) {
			return 1;
		}
	}
	return 0;
}

static int
archetypeCreateOne(struct Archetype *form, cJSON *entry)
{
	cJSON *count = cJSON_GetObjectItemCaseSensitive(entry, "count");
	cJSON *comps = cJSON_GetObjectItemCaseSensitive(entry, "components");

	if (!count || !cJSON_IsNumber(count)) {
		abort();
	}
	if (!comps || !cJSON_IsArray(comps)) {
		abort();
	}

	int c = count->valueint;

	form->max = c;
	form->cur = 0;
	form->space = spaceCreate(ENTT_CHUNK_SIZE, c);

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
	cJSON *root = readJson("archetypes");
	if (!root)
		return 1;

	cJSON *actor = cJSON_GetObjectItemCaseSensitive(root, "actor");
	archetypeCreateOne(&d->entt[ARCHETYPE_MONSTER], actor);

	cJSON *item = cJSON_GetObjectItemCaseSensitive(root, "item");
	archetypeCreateOne(&d->entt[ARCHETYPE_ITEM], item);

	cJSON_Delete(root);

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
entityJson(Entities all_types, const char *filename, Handle *out)
{
	HandleID id;

	cJSON *json = readJson(filename);
	if (!json)
		return 1;

	struct Archetype *a = NULL;

	// Archetype
	cJSON *archetype = cJSON_GetObjectItemCaseSensitive(json, "archetype");
	if (archetype && cJSON_IsString(archetype)) {
		if (strncmp(archetype->valuestring, "actor", strlen("actor")) == 0) {
			out->type = ARCHETYPE_MONSTER;
		} else if (!strncmp(archetype->valuestring, "item", strlen("item"))) {
			out->type = ARCHETYPE_ITEM;
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
	readJsonCopyString(json, "name", a->name, id);
	readJsonCopyChar(json, "glyph", a->glyph, id);
	
	readJsonCopyInt(json, "hp", a->hp, id);

	// Actor Stats
	readJsonCopyUint32(json, "str", a->str, id);
	readJsonCopyUint32(json, "con", a->con, id);
	readJsonCopyUint32(json, "per", a->per, id);
	readJsonCopyUint32(json, "dex", a->dex, id);
	readJsonCopyUint32(json, "wis", a->wis, id);
	
	// Weapon Stats
	readJsonCopyUint32(json, "attack", a->attack, id);
	readJsonCopyUint32(json, "range", a->range, id);
	readJsonCopyUint32(json, "damage", a->damage, id);

	// Inventory
	cJSON *inventory_c = cJSON_GetObjectItemCaseSensitive(json, "inventory");
	if (cJSON_IsNumber(inventory_c)) {
		VECTOR_CREATE(&a->inventory[id], inventory_c->valueint);
	}

	/*
	char *defence = NULL;
	jsonPeakString(json, "defence", &defence);
	if (defence) {
		if (strncmp(defence, "fortitude", strlen("fortitude")))
			a->def_type[han.id] = DEF_FORTITUDE;

		if (strncmp(defence, "will", strlen("will")))
			a->def_type[han.id] = DEF_WILL;

		if (strncmp(defence, "reflex", strlen("reflex")))
			a->def_type[han.id] = DEF_REFLEX;

		free(defence);
	}
	*/

	cJSON_Delete(json);
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

	// HandleID hit;
	// if (0 == spaceGet(type->space, &hit, delta)) {
	//		return;
	// }

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
	// int fortitude = fmax(def_entt->str[def.id], def_type->con[def.id]);
	//  todo add AC/weapons

	if (def_entt->hp[def.id] <= 0) {

		def_entt->glyph[def.id] = utf8Decomp("X");
	}

	return 0;
}
