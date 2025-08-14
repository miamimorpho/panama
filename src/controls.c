#include "controls.h"
#include "terminal.h"

int userInput(struct Dungeon *d, struct Mob* pla)
{
    switch (termIn()){
        case T_KEY_UP:
            mobMove(d, pla, VEC16_NORTH(pla->pos));
            break;
        case T_KEY_DOWN:
            mobMove(d, pla, VEC16_SOUTH(pla->pos));
            break;
        case T_KEY_LEFT:
            mobMove(d, pla, VEC16_WEST(pla->pos));
            break;
        case T_KEY_RIGHT:
            mobMove(d, pla, VEC16_EAST(pla->pos));
            break;
        default:
            return 0;
    }
    return 0;
}
