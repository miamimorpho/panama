#include "controls.h"
#include "terminal.h"

int userInput(struct Dungeon *d, Monster pla)
{
    vec16 start, next;
    monsterPos(d, pla, start);
    vec16Copy(start, next);

    switch (termIn()){
        case T_KEY_UP:
            vec16Copy(VEC16_NORTH(start), next);
            break;
        case T_KEY_DOWN:
            vec16Copy(VEC16_SOUTH(start), next);
            break;
        case T_KEY_LEFT:
            vec16Copy(VEC16_WEST(start), next);
            break;
        case T_KEY_RIGHT:
            vec16Copy(VEC16_EAST(start), next);
            break;
        default:
            return 1;
    }

    monsterMove(d, pla, next);
    return 0;
}
