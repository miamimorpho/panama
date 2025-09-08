#include "controls.h"
#include "terminal.h"
#include "menu.h"

int
userInput(struct Dungeon *d, Handle pla)
{
	vec16 start, next;
	entityWhere(d->entt, pla, start);
	vec16Copy(start, next);

	switch (termGet()) {
	case T_KEY_UP:
		vec16Copy(VEC16_NORTH(start), next);
		break;
	case T_KEY_DOWN:
		vec16Copy(VEC16_SOUTH(start), next);
		break;
	case T_KEY_LEFT:
		vec16Copy(VEC16_WEST(start), next);
		break;
	case T_KEY_RIGHT:
		vec16Copy(VEC16_EAST(start), next);
		break;
	case 'g':
		// monsterPickupItem(d, pla);
		break;
	// case 'i':
	//	menuInventory(d, d->monsters->inventory[pla.id]);
	//	break;
	default:
		return 1;
	}

	HandleID hit;
	if (0 == entityGet(d->entt, ARCHETYPE_MONSTER, next, &hit)) {
		// monster hit
	}

	entityMove(d->entt, pla, next);

	return 0;
}
