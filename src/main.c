
#include "terminal.h"
#include "controls.h"
#include "ai.h"
#include "render.h"

int
main(void)
{
	termInit();

	struct Dungeon d;
	dungeonCreate(&d);
	dungeonGenerate(&d);

	Handle player;
	entityCreate(d.entt, ARCHETYPE_MONSTER, (vec16) {4, 4}, &player);
	entityJson(d.entt, "player", player);

	Handle goblin;
	entityCreate(d.entt, ARCHETYPE_MONSTER, (vec16) {16, 16}, &goblin);
	entityJson(d.entt, "goblin", goblin);

	Handle sword;
	entityCreate(d.entt, ARCHETYPE_ITEM, (vec16) {6, 6}, &sword);
	entityJson(d.entt, "sword", sword);

	while (1) {
		termClear();
		vec16 player_pos;
		entityWhere(d.entt, player, player_pos);
		drawDungeon(&d, player_pos);
		termFlush();
		if (userInput(&d, player))
			continue;
		monsterAI(&d, goblin, player);
	}
}
