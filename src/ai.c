#include "ai.h"
#include "astar.h"

void
monsterAI(struct Dungeon *d, Handle m, Handle pla)
{
	if (entityIsDead(d->entt, m))
		return;

	vec16 path[1024];
	unsigned int path_len = 1024;
	struct AStar *aa;

	vec16 p1, p2;
	entityWhere(d->entt, m, p1);
	entityWhere(d->entt, pla, p2);
	aStar(d, p2, p1, &aa);
	aStarBuildPath(aa, NULL, &path_len);
	aStarBuildPath(aa, path, &path_len);
	if (path_len) {
		vec16 p3;
		vec16Add(p1, path[1], p3);
		Handle hit = entityGet(d->entt, ARCHETYPE_MONSTER, p3);

		if (hit.type == ARCHETYPE_NONE)
			entityMove(d, m, path[1]);
	}
}
