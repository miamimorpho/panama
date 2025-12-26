#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "json.h"

int
readJsonCopyChar(cJSON *root, const char *key, struct TermTile *array,
				 size_t index)
{
	if (!array)
		return 0; // Skip if array is NULL
	if (!root || !key)
		return 1;

	cJSON *elm = cJSON_GetObjectItemCaseSensitive(root, key);
	if (!elm || !cJSON_IsString(elm) || !elm->valuestring ||
		elm->valuestring[0] == '\0') {
		return 1;
	}
	array[index].utf = utf8Decomp(elm->valuestring);
	return 0;
}

int
readJsonCopyInt(cJSON *root, const char *key, int *array, size_t index)
{
	if (!array)
		return 0; // Skip if array is NULL
	if (!root || !key)
		return 1;

	cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);
	if (!item || !cJSON_IsNumber(item)) {
		return 1;
	}
	array[index] = item->valueint;
	return 0;
}

int
readJsonCopyUint32(cJSON *root, const char *key, uint32_t *array, size_t index)
{
	if (!array)
		return 0; // Skip if array is NULL
	if (!root || !key)
		return 1;

	cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);
	if (!item || !cJSON_IsNumber(item)) {
		return 1;
	}
	if (item->valueint < 0)
		return 1; // Negative values invalid for uint32_t

	array[index] = (int32_t) item->valueint;
	return 0;
}

int
jsonPeakString(cJSON *root, const char *key, char **array)
{
	if (!array)
		return 0; // Skip if array is NULL
	if (!root || !key)
		return 1;

	cJSON *elm = cJSON_GetObjectItemCaseSensitive(root, key);
	if (!elm || !cJSON_IsString(elm) || !elm->valuestring) {
		return 1;
	}

	size_t len = strlen(elm->valuestring) + 1;
	*array = calloc(len, sizeof(char));
	if (!*array)
		return 1; // Memory allocation failed

	memcpy(*array, elm->valuestring, len);
	return 0;
}

int
readJsonCopyString(cJSON *root, const char *key, char **array, size_t index)
{
	return jsonPeakString(root, key, &array[index]);
}

cJSON *
readJson(const char *name)
{
	char filepath[256];
	snprintf(filepath, sizeof(filepath), "./json/%s.json", name);
	FILE *f = fopen(filepath, "r");
	if (!f) {
		fprintf(stderr, "Failed to open file %s\n", filepath);
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	long filesize = ftell(f);
	if (filesize < 0) {
		fclose(f);
		fprintf(stderr, "Failed to get file size for %s\n", filepath);
		return NULL;
	}
	fseek(f, 0, SEEK_SET);
	char *data = malloc(filesize + 1);
	if (!data) {
		fclose(f);
		fprintf(stderr, "Memory allocation failed\n");
		return NULL;
	}
	size_t bytes_read = fread(data, 1, filesize, f);
	fclose(f);
	if (bytes_read != (size_t) filesize) {
		free(data);
		fprintf(stderr, "Failed to read complete file %s\n", filepath);
		return NULL;
	}
	data[filesize] = '\0'; // null-terminate string
	cJSON *json = cJSON_Parse(data);
	free(data);
	if (!json) {
		fprintf(stderr, "Failed to parse JSON for %s: %s\n", filepath,
				cJSON_GetErrorPtr());
		return NULL;
	}
	return json;
}
