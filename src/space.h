#ifndef SPACE_H
#define SPACE_H

#include "ivec16.h"
#include "entity.h"

struct Space;

struct SpaceFinder {
	struct Space *s;

	// everything is in quantised coordinates
	vec16 origin;
	uint32_t range;
	uint32_t cur;
	uint32_t total;
};

struct Space *spaceCreate(int len, int c);
int spaceGet(struct Space *, HandleID *, vec16);
void spaceInsert(struct Space *, HandleID, vec16);
void spaceRemove(struct Space *, HandleID);
void spaceMove(struct Space *, HandleID, vec16);
int spaceWhere(struct Space *, HandleID, vec16);

HandleID spaceFindStart(struct Space *, struct SpaceFinder *, vec16, uint32_t);
HandleID spaceFindNext(struct SpaceFinder *f, HandleID);
#define SPACE_FIND(space, origin, range, finder, handle)                       \
	for (handle = spaceFindStart(space, &finder, origin, range);               \
		 handle != NULL_HANDLE; handle = spaceFindNext(&finder, handle))

#endif // SPACE_H
