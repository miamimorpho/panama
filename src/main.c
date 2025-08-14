
#include "terminal.h"
#include "render.h"
#include "controls.h"
#include "dungeon.h"
#include "dungeon_mem.h"
#include "astar.h"

int main(void) {
	termInit();

    struct Dungeon *d = dungeonCreate();
    dungeonGenerate(d);

    struct Mob *player = mobCreate(d, (vec16){4, 4});
    player->ch = '@';

    struct Mob *goblin = mobCreate(d, (vec16){10, 10});
    goblin->ch = 'g';

    vec16 path[16];
    uint32_t path_len = 16;
    struct AStar *aa;

    while(1){
        aStar(d, goblin->pos, player->pos, &aa);
        aStarBuildPath(aa, path, &path_len);
        mobMove(d, goblin, path[path_len -1]);

        drawMobs(d, player->pos);
        termRefresh();
        userInput(d, player);
    }
}
