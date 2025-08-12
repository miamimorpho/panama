#include <stdint.h>

struct PQue;
struct PQueElm;

int PQueInsert(struct PQue *, uint32_t, uint32_t, double, double);
int PQuePop(struct PQue *, struct PQueElm*);
int PQueIsDone(struct PQue *q, uint32_t val);
