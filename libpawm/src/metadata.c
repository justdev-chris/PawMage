// libpawm/src/metadata.c
#include "../include/pawm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ========== MINIMAL JSON PARSER ==========
char* pawm_metadata_get(const char* metadata, const char* key) {
    if (!metadata || !key) return NULL;

    char* pattern = (char*)malloc(strlen(key) + 4);
    sprintf(pattern, "\"%s\":", key);

    char* found = strstr(metadata, pattern);
    free(pattern);
    if (!found) return NULL;

    char* start = strchr(found + strlen(key) + 2, '"');
    if (!start) return NULL;
    start++;

    char* end = strchr(start, '"');
    if (!end) return NULL;

    size_t len = end - start;
    char* value = (char*)malloc(len + 1);
    memcpy(value, start, len);
    value[len] = '\0';

    return value;
}

// ========== SET METADATA ==========
char* pawm_metadata_set(const char* metadata, const char* key, const char* value) {
    char* old_value = pawm_metadata_get(metadata, key);

    if (old_value) {
        free(old_value);
        char pattern[512];
        snprintf(pattern, sizeof(pattern), "\"%s\":\"%s\"", key, old_value);
        char* pos = strstr(metadata, pattern);
        if (!pos) return NULL;

        size_t prefix_len = pos - metadata;
        size_t suffix_start = (size_t)(pos - metadata) + strlen(pattern);
        
        char* result = (char*)malloc(strlen(metadata) + strlen(key) + strlen(value) + 10);
        sprintf(result, "%.*s\"%s\":\"%s\"%s", 
                (int)prefix_len, metadata, key, value, metadata + suffix_start);
        return result;
    } else {
        char* result;
        if (strcmp(metadata, "{}") == 0) {
            result = (char*)malloc(strlen(key) + strlen(value) + 10);
            sprintf(result, "{\"%s\":\"%s\"}", key, value);
        } else {
            result = (char*)malloc(strlen(metadata) + strlen(key) + strlen(value) + 10);
            sprintf(result, "%.*s,\"%s\":\"%s\"}", 
                    (int)(strlen(metadata) - 1), metadata, key, value);
        }
        return result;
    }
}

// ========== CREATE DEFAULT METADATA ==========
char* pawm_metadata_default() {
    return strdup("{\"format\":\"pawm\",\"version\":\"1\"}");
}

// ========== METADATA HELPERS ==========

void pawm_metadata_set_author(PawMageImage* img, const char* author) {
    if (!img) return;
    char* new_meta = pawm_metadata_set(img->metadata, "author", author);
    if (new_meta) {
        free(img->metadata);
        img->metadata = new_meta;
        img->metadata_len = strlen(new_meta);
    }
}

void pawm_metadata_set_description(PawMageImage* img, const char* desc) {
    if (!img) return;
    char* new_meta = pawm_metadata_set(img->metadata, "description", desc);
    if (new_meta) {
        free(img->metadata);
        img->metadata = new_meta;
        img->metadata_len = strlen(new_meta);
    }
}

void pawm_metadata_set_timestamp(PawMageImage* img) {
    if (!img) return;
    char timestamp[32];
    sprintf(timestamp, "0");
    char* new_meta = pawm_metadata_set(img->metadata, "timestamp", timestamp);
    if (new_meta) {
        free(img->metadata);
        img->metadata = new_meta;
        img->metadata_len = strlen(new_meta);
    }
}

char* pawm_metadata_get_author(PawMageImage* img) {
    if (!img) return NULL;
    return pawm_metadata_get(img->metadata, "author");
}

char* pawm_metadata_get_description(PawMageImage* img) {
    if (!img) return NULL;
    return pawm_metadata_get(img->metadata, "description");
}
