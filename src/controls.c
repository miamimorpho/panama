#include "controls.h"
#include "terminal.h"

int userInput(struct Dungeon *d, struct Mob* player)
{
    switch (termIn()){
        case T_KEY_UP:
            mobMove(d, player, 0, -1);
            break;
        case T_KEY_DOWN:
            mobMove(d, player, 0, 1);
            break;
        case T_KEY_LEFT:
            mobMove(d, player, -1, 0);
            break;
        case T_KEY_RIGHT:
            mobMove(d, player, 1, 0);
            break;
        default:
            return 0;
    }
    return 0;
}
