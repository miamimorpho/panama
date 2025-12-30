#include <stdint.h>
#include "../extern/json.h"
#include "terminal_types.h"

json_value *jsonReadFile(const char *name);

json_value *jsonFindField(json_value *, const char *);
const char *jsonGetString(json_value *, const char *);
const long long *jsonGetInt(json_value *, const char *);
int jsonArrayHasEntry(json_value *, const char *);
