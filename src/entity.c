#include <assert.h>
#include <string.h>

#include "terra.h"
#include "entity.h"
#include "space.h"
#include "json.h"
#include <math.h>

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

    // MetaData
	readJsonCopyString(json, "name", a->names, han.id);
	readJsonCopyChar(json, "tile", a->tiles, han.id);
	readJsonCopyInt(json, "hp", a->hp, han.id);
	
    // Actor Stats
	readJsonCopyUint32(json, "str", a->str, han.id);
    readJsonCopyUint32(json, "con", a->con, han.id);
    readJsonCopyUint32(json, "per", a->per, han.id);
    readJsonCopyUint32(json, "dex", a->dex, han.id);
    readJsonCopyUint32(json, "wis", a->wis, han.id);
  
    // Weapon Stats
    readJsonCopyUint32(json, "bonus", a->bonus, han.id);
    readJsonCopyUint32(json, "range", a->range, han.id);
    readJsonCopyUint32(json, "damage", a->damage, han.id);
    char *defence = NULL;
    jsonPeakString(json, "defence", &defence);
    if(defence){
        if(strncmp(defence, "fortitude", strlen("fortitude")))
            a->def_type[han.id] = DEF_FORTITUDE;
    
        if(strncmp(defence, "will", strlen("will")))
            a->def_type[han.id] = DEF_WILL;
    
        if(strncmp(defence, "reflex", strlen("reflex")))
            a->def_type[han.id] = DEF_REFLEX;
   
        free(defence);
    }
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

	HandleID hit;
	if (0 == spaceGet(type->space, &hit, delta)) {
		return;
	}

	spaceMove(type->space, in.id, delta);
}

int
entityPickUp(Entities all_types, Handle grabber, Handle item)
{
	struct Archetype *gra_t = &all_types[grabber.type];
	struct Archetype *itm_t = &all_types[item.type];

	if (!itm_t->inventory_host)
		return 1;
	if (!gra_t->inventory)
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
    //int fortitude = fmax(def_entt->str[def.id], def_type->con[def.id]);
    // todo add AC/weapons

	if (def_entt->hp[def.id] <= 0) {

		def_entt->tiles[def.id].utf = utf8Char("X");
	}

	return 0;
}
