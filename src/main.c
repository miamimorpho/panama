
#include "terminal.h"
#include "render.h"
#include "controls.h"
#include "dungeon.h"
#include "dungeon_mem.h"

int main(void) {
	termInit();

    struct Dungeon *d = dungeonCreate();
    dungeonGenerate(d);

    struct Mob *player = mobCreate(d, 4, 4);
    player->ch = '@';

    struct Mob *goblin = mobCreate(d, 6, 6);
    goblin->ch = 'g';

    while(1){
        drawMobs(d, player->pos.x, player->pos.y);
        termRefresh();
        userInput(d, player);
    }
}
