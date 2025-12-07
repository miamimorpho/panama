#include "controls.h"
#include "terminal.h"
#include "menu.h"

int
userInput(struct Dungeon *d, Handle pla)
{
	vec16 start, next;
	entityWhere(d->entt, pla, start);
	vec16Copy(start, next);

	Handle gettee = {0};

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
		gettee = entityGet(d->entt, ARCHETYPE_ITEM, start);
		entityPickUp(d->entt, pla, gettee);
		return 0;
		break;
	case 'i':
		return menuInventory(d, &d->entt[pla.type].inventory[pla.id]);
		break;
	default:
		return 1;
	}

	Handle hit = entityGet(d->entt, ARCHETYPE_MONSTER, next);
	if (hit.type == ARCHETYPE_MONSTER) {
		if (d->entt[hit.type].hp[hit.id] >= 0) {
			entityAttack(d->entt, pla, hit);
			return 0;
		}
	}

	entityMove(d, pla, next);

	return 0;
}
