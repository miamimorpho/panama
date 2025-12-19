
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
	dungeonGenerate(&d, "dungeon");

	Handle player;
	entityJson(d.entt, "player", &player);

	Handle goblin;
	entityJson(d.entt, "goblin", &goblin);
	entityMove(&d, goblin, (vec16) {5, 5});

	Handle sword;
	entityJson(d.entt, "sword", &sword);

	while (1) {
		
		vec16 player_pos;
		entityWhere(d.entt, player, player_pos);

		drawDungeon(&d, player_pos);
		termFlush();

		if (userInput(&d, player))
			continue;

		monsterAI(&d, goblin, player);
	}
}
