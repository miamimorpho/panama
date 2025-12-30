#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>

#include "json_wrapper.h"
#include "error.h"

json_value *
jsonFindField(json_value *root, const char *key)
{
	assert(root && root->type == json_object);

	for (size_t i = 0; i < root->u.object.length; i++) {
		if (strcmp(root->u.object.values[i].name, key) == 0) {
			return root->u.object.values[i].value;
		}
	}

	return NULL;
}

const char *
jsonGetString(json_value *j, const char *key)
{
	json_value *elm = jsonFindField(j, key);
	if (elm && elm->type == json_string && elm->u.string.ptr) {
		return elm->u.string.ptr;
	}
	// fprintf(stderr, "WARN jsonGetString(... \"%s\") -> ?default\n", key);
	return NULL;
}

const long long *
jsonGetInt(json_value *j, const char *key)
{
	static long long ret = 0;

	json_value *elm = jsonFindField(j, key);
	if (elm && elm->type == json_integer && elm->u.integer) {
		return &elm->u.integer;
	}
	return NULL;
}

int
jsonArrayHasEntry(json_value *j, const char *comp_name)
{
	if (!j || j->type != json_array)
		return 0;

	for (size_t i = 0; i < j->u.array.length; i++) {
		json_value *item = j->u.array.values[i];
		if (item->type == json_string &&
			item->u.string.length >= strlen(comp_name) &&
			strncmp(item->u.string.ptr, comp_name, strlen(comp_name)) == 0) {
			return 1;
		}
	}
	return 0;
}

json_value *
jsonReadFile(const char *name)
{
	char filepath[256];
	snprintf(filepath, sizeof(filepath), "./json/%s.json", name);

	FILE *fp;
	struct stat filestatus;
	int file_size;
	char *file_contents;
	json_char *json;
	json_value *value;

	if (stat(filepath, &filestatus) != 0) {
		fprintf(stderr, "jsonReadFile(\"%s\") -> File not found\n", filepath);
		return NULL;
	}
	file_size = filestatus.st_size;
	file_contents = (char *) malloc(filestatus.st_size);
	if (file_contents == NULL) {
		fprintf(stderr, "Memory error: unable to allocate %d bytes\n",
				file_size);
		return NULL;
	}

	fp = fopen(filepath, "rt");
	if (fp == NULL) {
		fprintf(stderr, "Unable to open %s\n", filepath);
		fclose(fp);
		free(file_contents);
		return NULL;
	}
	if (fread(file_contents, file_size, 1, fp) != 1) {
		fprintf(stderr, "Unable to read content of %s\n", filepath);
		fclose(fp);
		free(file_contents);
		return NULL;
	}
	fclose(fp);

	json = (json_char *) file_contents;
	value = json_parse(json, file_size);

	if (value == NULL) {
		fprintf(stderr, "Unable to parse data\n");
		free(file_contents);
		exit(1);
	}

	free(file_contents);

	return value;
}
