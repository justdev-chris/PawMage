// libpawm/include/pawm.h
#ifndef PAWM_H
#define PAWM_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PAWM_MAGIC "PAWM"
#define PAWM_VERSION 1

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t depth;      // 1, 8, 24, 32
    uint32_t metadata_len;
    char* metadata;     // JSON string
    uint8_t* pixels;    // raw pixel data
} PawMageImage;

// Core functions
PawMageImage* pawm_load(const char* path);
PawMageImage* pawm_load_from_memory(const uint8_t* data, size_t size);
int pawm_save(const char* path, PawMageImage* img);
void pawm_free(PawMageImage* img);
uint32_t pawm_get_pixel(PawMageImage* img, uint32_t x, uint32_t y);
void pawm_set_pixel(PawMageImage* img, uint32_t x, uint32_t y, uint32_t color);

// Metadata functions
char* pawm_metadata_get(const char* metadata, const char* key);
char* pawm_metadata_set(const char* metadata, const char* key, const char* value);
char* pawm_metadata_default(void);
void pawm_metadata_set_author(PawMageImage* img, const char* author);
void pawm_metadata_set_description(PawMageImage* img, const char* desc);
void pawm_metadata_set_timestamp(PawMageImage* img);
char* pawm_metadata_get_author(PawMageImage* img);
char* pawm_metadata_get_description(PawMageImage* img);

#ifdef __cplusplus
}
#endif

#endif