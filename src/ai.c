#include "ai.h"
#include "astar.h"

void monsterAI(struct Dungeon *d, Monster m, Monster pla){

    if(d->monsters->stats[m].hp <= 0)
        return;

    vec16 path[1024];
    unsigned int path_len = 1024;
    struct AStar *aa;

    vec16 p1, p2;
    monsterWhere(d, m, p1);
    monsterWhere(d, pla, p2);
    aStar(d, p1, p2, &aa);
    aStarBuildPath(aa, NULL, &path_len);
    aStarBuildPath(aa, path, &path_len);
    if(path_len){
        monsterMove(d, m, path[path_len -1], NULL);
    }
}
