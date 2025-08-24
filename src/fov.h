#include "dungeon.h"
#include "space.h"

struct FovEffect;
typedef void (*FovEffectFn)(struct FovEffect *, struct TerraPos, vec16);

struct FovEffect{
    FovEffectFn fn;
    void *ctx;
};

void fov(struct Dungeon *d, vec16 o, struct FovEffect *effect);
