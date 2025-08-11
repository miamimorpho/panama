#include <stdint.h>

struct PQue;

int PQueInsert(struct PQue *, uint32_t, double);
int PQuePop(struct PQue *, uint32_t *, double *);
int PQueIsDone(struct PQue *q, uint32_t val);
