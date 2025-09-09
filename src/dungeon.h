#ifndef DUNGEON_H
#define DUNGEON_H

#include "terminal_types.h"
#include "ivec16.h"

struct Dungeon;

// typedef int (*ObjCreateFn)(void *, HandleID *);
// typedef int (*ObjDerefFn)(void *, HandleID *, Handle);

/* turns a position into a unique handleID
 */
// typedef int (*ObjGetFn)(void *, HandleID *, vec16);
/* inverse of get, turns a unique handle to a position
 */
// typedef int (*ObjWhereFn)(void *, HandleID *, vec16);
// typedef int (*ObjMoveFn)(void *, HandleID *, vec16);
// typedef int (*ObjTileFn)(void *, HandleID *, char **);
// typedef int (*ObjIsOpaque)(void *, HandleID *);
// typedef int (*ObjIsSolid)(void *, HandleID *);
// typedef int (*ObjMoveInto)(void *, HandleID *, Handle);
/*
struct VTable {
	ObjIsOpaque is_opaque;
	ObjIsSolid is_solid;
};
*/

void dungeonCreate(struct Dungeon *);
void dungeonGenerate(struct Dungeon *);

#endif // DUNGEON_H
