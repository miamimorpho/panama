#include "items.h"
// #include "terminal_types.h"
#include "space.h"

static const int ITEM_MAX_C = 128;
static const int LOOKUP_GRANULARITY = 16;

int
itemsCreate(struct Archetype *a)
{
	a->max = ITEM_MAX_C;
	a->cur = 0;

	a->space = spaceCreate(LOOKUP_GRANULARITY, ITEM_MAX_C);

	COMPONENT_ADD(a, names, ITEM_MAX_C);
	COMPONENT_ADD(a, tiles, ITEM_MAX_C);
	COMPONENT_ADD(a, container_host, ITEM_MAX_C);
	COMPONENT_ADD(a, container_arr, ITEM_MAX_C);
	COMPONENT_ADD(a, atk, ITEM_MAX_C);
	COMPONENT_ADD(a, def, ITEM_MAX_C);
	COMPONENT_ADD(a, hp, ITEM_MAX_C);

	return 0;
}
