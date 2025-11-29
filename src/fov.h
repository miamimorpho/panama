#include "dungeon.h"
#include "terra.h"
#include "bitmap.h"

struct FovEffect;
typedef void (*FovEffectFn)(struct FovEffect *, struct TerraPos, vec16);

struct FovEffect {
	FovEffectFn fn;
	void *ctx;
};

void fov(struct Dungeon *, vec16, struct FovEffect *);
