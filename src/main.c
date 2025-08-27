
#include "terminal.h"
#include "controls.h"
#include "items.h"
#include "ai.h"
#include "render.h"

int main(void) {
	termInit();

    struct Dungeon d = {0};
    d.space = spaceCreate();
    d.monsters = monsterManCreate();
    d.items = itemManCreate();
    dungeonGenerate(&d);

    Monster player;
    monsterCreate(&d, (vec16){4, 4}, &player);
    monsterJSONRead(d.monsters, "player", player);
    
    Monster goblin;
    monsterCreate(&d, (vec16){16, 16}, &goblin);
    monsterJSONRead(d.monsters, "goblin", goblin);
    
    struct ItemStats candle_stat = {0};
    Item candle;
    itemCreate(&d, (vec16){6, 6}, 
            "candle",
            'c',
            candle_stat,
            &candle);
    
    while(1){
        vec16 player_pos;
        monsterWhere(&d, player, player_pos);

        drawDungeon(&d, player_pos);
        termRefresh();
        userInput(&d, player);

        monsterAI(&d, goblin, player);
    }
}
