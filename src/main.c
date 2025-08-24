
#include "terminal.h"
#include "controls.h"
#include "monster.h"
#include "space.h"
#include "astar.h"
#include "render.h"

int main(void) {
	termInit();


    struct Dungeon d = {0};
    d.space = spaceCreate();
    d.monsters = monsterManCreate();
    dungeonGenerate(&d);

    Monster player = monsterCreate(&d, (vec16){4, 4});
    *monsterTile(&d, player) = '@';
    
    Monster goblin = monsterCreate(&d, (vec16){16, 16});
    *monsterTile(&d, goblin) = 'g';
    
    vec16 path[1024];
    uint32_t path_len = 16;
    struct AStar *aa;

    while(1){
        path_len = 1024;
        vec16 p1, p2;
        monsterPos(&d, goblin, p1);
        monsterPos(&d, player, p2);
        aStar(&d, p1, p2, &aa);
        aStarBuildPath(aa, NULL, &path_len);
        aStarBuildPath(aa, path, &path_len);
        if(path_len)
            monsterMove(&d, goblin, path[path_len -1]);

        drawDungeon(&d, p2);
        termRefresh();
        userInput(&d, player);
    }
}
