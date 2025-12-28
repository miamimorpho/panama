#include "ivec16.h"
#include "terminal_types.h"

struct Terrain;

struct TerraTile {
	utf8_ch utf;
	enum ColorSym fg;
	enum ColorSym bg;	
};

struct TerraPos {
	struct TerraChunk *chunk;
	vec16 pos;
};

struct Terrain *terrainCreate(void);
struct TerraPos terraPosNew(struct Terrain *, vec16);
struct TerraPos terraPos(struct Terrain *, vec16);
void terraPutOpaque(struct TerraPos, bool);
bool terraGetOpaque(struct TerraPos);
void terraPutSolid(struct TerraPos, bool);
bool terraGetSolid(struct TerraPos);
void terraPutFog(struct TerraPos, bool);
bool terraGetFog(struct TerraPos);

void terraPutTile(struct TerraPos, struct TerraTile);
struct TerraTile terraGetTile(struct TerraPos);

