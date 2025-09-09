typedef int (*ObjCreateFn)(void *, HandleID *);
typedef int (*ObjJsonFn)(void *, HandleID *, const char *);
/* turns a struct { archetype, id } to a unique handleID for that type
 */
typedef int (*ObjDerefFn)(void *, HandleID *, Handle);

/* turns a position into a unique handleID
 */
typedef int (*ObjGetFn)(void *, HandleID *, vec16);
/* inverse of get, turns a unique handle to a position
 */
typedef int (*ObjWhereFn)(void *, HandleID *, vec16);
typedef int (*ObjMoveFn)(void *, HandleID *, vec16);
typedef int (*ObjTileFn)(void *, HandleID *, char **);
typedef int (*ObjIsOpaque)(void *, HandleID *);
typedef int (*ObjIsSolid)(void *, HandleID *);
typedef int (*ObjMoveInto)(void *, HandleID *, Handle);

struct VTable {
	ObjDerefFn deref;
	ObjGetFn get;
	ObjCreateFn create;
	ObjJsonFn json;
	ObjWhereFn where;
	ObjTileFn tile;
};
