#include <stdint.h>

#include "../extern/cJSON.h"
#include "terminal_types.h"

int readJsonCopyChar(cJSON *, const char *, utf8_ch *, size_t);
int readJsonCopyInt(cJSON *root, const char *key, int *array, size_t index);
int readJsonCopyUint32(cJSON *root, const char *key, uint32_t *array,
					   size_t index);

int jsonPeakString(cJSON *root, const char *key, char **array);
int readJsonCopyString(cJSON *root, const char *key, char **array,
					   size_t index);

// JSON file handling
cJSON *readJson(const char *name);

// Enti
