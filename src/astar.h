#include <stdint.h>
#include "dungeon_mem.h"

struct AStar;

int aStar(struct Dungeon *d, vec16 start, vec16 goal, struct AStar **a);
int aStarBuildPath(struct AStar *a, vec16 *path, uint32_t *path_len);
