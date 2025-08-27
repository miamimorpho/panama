#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reader.h"

int
readJsonCopyChar(cJSON *root, const char *key, utf32_t *dst)
{
    if (!root || !key || !dst) {
        return 1;
    }
   
    cJSON *elm = cJSON_GetObjectItemCaseSensitive(root, key);
    if (!elm || !cJSON_IsString(elm) || 
            !elm->valuestring || elm->valuestring[0] == '\0') {
        return 1;
    }
        
    *dst = elm->valuestring[0];   
    return 0;
}

// Read integer from JSON object by key
int readJsonCopyInt(cJSON *root, const char *key, int *dst) {
    if (!root || !key || !dst) {
        return 1;
    }
     
    cJSON *item = cJSON_GetObjectItemCaseSensitive(root, key);
    if (!item || !cJSON_IsNumber(item)) {
        return 1;
    }
    
    *dst = item->valueint;
    return 0;
}

int 
readJsonCopyString(cJSON *root, 
        const char* key,
        char **dst, 
        size_t *out)
{
    if (!root || !key ) {
        return 1;
    }
     
    cJSON *elm = cJSON_GetObjectItemCaseSensitive(root, key);
    if (!elm || !cJSON_IsString(elm) || !elm->valuestring) {
        return 1;
    }

    size_t len = strlen(elm->valuestring) + 1; // include null terminator

    if(out)
        *out = len;
    else
        return 1;
    
    if(dst)
        memcpy(*dst, elm->valuestring, len);
    
    return 0;
}

cJSON *readJson(const char *name) {
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
    
    if (bytes_read != (size_t)filesize) {
        free(data);
        fprintf(stderr, "Failed to read complete file %s\n", filepath);
        return NULL;
    }
    
    data[filesize] = '\0';  // null-terminate string
    
    cJSON *json = cJSON_Parse(data);
    free(data);
    
    if (!json) {
        fprintf(stderr, "Failed to parse JSON for %s: %s\n", filepath, cJSON_GetErrorPtr());
        return NULL;
    }
    
    return json;
}

