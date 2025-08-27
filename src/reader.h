#include "cJSON.h"
#include "utf32.h"

cJSON *readJson(const char *);
int readJsonCopyString(cJSON *, const char*, char **, size_t *);
int readJsonCopyChar(cJSON *, const char*, utf32_t *);
int readJsonCopyInt(cJSON *, const char *, int *);

