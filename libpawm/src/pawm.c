// libpawm/src/pawm.c
#include "../include/pawm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ========== INTERNAL HEADER STRUCT ==========
typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t  depth;
    uint32_t metadata_len;
    uint32_t compressed_len;   // NEW: size of compressed pixel data
} PawMageHeader;

// ========== FORWARD DECLS ==========
uint8_t* rle_compress(const uint8_t* input, size_t input_len, size_t* out_len);
uint8_t* rle_decompress(const uint8_t* input, size_t input_len, size_t* out_len);

// ========== LOAD FROM FILE ==========
PawMageImage* pawm_load(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* buffer = (uint8_t*)malloc(size);
    fread(buffer, 1, size, f);
    fclose(f);

    PawMageImage* img = pawm_load_from_memory(buffer, size);
    free(buffer);
    return img;
}

// ========== LOAD FROM MEMORY ==========
PawMageImage* pawm_load_from_memory(const uint8_t* data, size_t size) {
    if (size < 20) return NULL; // 4 magic + 4+4+1+4+4 = 21 bytes min

    if (memcmp(data, "PAWM", 4) != 0) return NULL;

    const uint8_t* ptr = data + 4;

    PawMageHeader hdr;
    memcpy(&hdr.width, ptr, 4); ptr += 4;
    memcpy(&hdr.height, ptr, 4); ptr += 4;
    hdr.depth = *ptr; ptr += 1;
    memcpy(&hdr.metadata_len, ptr, 4); ptr += 4;
    memcpy(&hdr.compressed_len, ptr, 4); ptr += 4;

    if (hdr.width == 0 || hdr.height == 0) return NULL;
    if (hdr.depth != 1 && hdr.depth != 8 && hdr.depth != 24 && hdr.depth != 32) return NULL;

    PawMageImage* img = (PawMageImage*)malloc(sizeof(PawMageImage));
    img->width = hdr.width;
    img->height = hdr.height;
    img->depth = hdr.depth;
    img->metadata_len = hdr.metadata_len;

    img->metadata = (char*)malloc(hdr.metadata_len + 1);
    memcpy(img->metadata, ptr, hdr.metadata_len);
    img->metadata[hdr.metadata_len] = '\0';
    ptr += hdr.metadata_len;

    // Decompress pixels
    size_t pixel_bytes = hdr.width * hdr.height * (hdr.depth / 8);
    img->pixels = rle_decompress(ptr, hdr.compressed_len, &pixel_bytes);
    if (!img->pixels) {
        free(img->metadata);
        free(img);
        return NULL;
    }

    return img;
}

// ========== SAVE TO FILE ==========
int pawm_save(const char* path, PawMageImage* img) {
    if (!img || !img->pixels) return 0;

    size_t pixel_bytes = img->width * img->height * (img->depth / 8);
    size_t compressed_len = 0;
    uint8_t* compressed = rle_compress(img->pixels, pixel_bytes, &compressed_len);
    if (!compressed) return 0;

    FILE* f = fopen(path, "wb");
    if (!f) {
        free(compressed);
        return 0;
    }

    fwrite("PAWM", 1, 4, f);
    fwrite(&img->width, 4, 1, f);
    fwrite(&img->height, 4, 1, f);
    fwrite(&img->depth, 1, 1, f);
    fwrite(&img->metadata_len, 4, 1, f);
    fwrite(&compressed_len, 4, 1, f);

    if (img->metadata_len > 0) {
        fwrite(img->metadata, 1, img->metadata_len, f);
    }

    fwrite(compressed, 1, compressed_len, f);
    fclose(f);
    free(compressed);
    return 1;
}

// ========== FREE ==========
void pawm_free(PawMageImage* img) {
    if (!img) return;
    if (img->metadata) free(img->metadata);
    if (img->pixels) free(img->pixels);
    free(img);
}

// ========== GET PIXEL ==========
uint32_t pawm_get_pixel(PawMageImage* img, uint32_t x, uint32_t y) {
    if (!img || x >= img->width || y >= img->height) return 0;

    int bpp = img->depth / 8;
    uint8_t* p = img->pixels + (y * img->width + x) * bpp;

    switch (img->depth) {
        case 32: return p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24);
        case 24: return p[0] | (p[1]<<8) | (p[2]<<16) | 0xFF000000;
        case 8:  return p[0] | (p[0]<<8) | (p[0]<<16) | 0xFF000000;
        case 1:  return (p[x/8] & (1<<(7-x%8))) ? 0xFFFFFFFF : 0xFF000000;
        default: return 0;
    }
}

// ========== SET PIXEL ==========
void pawm_set_pixel(PawMageImage* img, uint32_t x, uint32_t y, uint32_t color) {
    if (!img || x >= img->width || y >= img->height) return;

    int bpp = img->depth / 8;
    uint8_t* p = img->pixels + (y * img->width + x) * bpp;

    switch (img->depth) {
        case 32:
            p[0] = color & 0xFF;
            p[1] = (color>>8) & 0xFF;
            p[2] = (color>>16) & 0xFF;
            p[3] = (color>>24) & 0xFF;
            break;
        case 24:
            p[0] = color & 0xFF;
            p[1] = (color>>8) & 0xFF;
            p[2] = (color>>16) & 0xFF;
            break;
        case 8:
            p[0] = (uint8_t)(((color & 0xFF) * 0.299) + 
                            (((color>>8) & 0xFF) * 0.587) + 
                            (((color>>16) & 0xFF) * 0.114));
            break;
        case 1:
            // For 1-bit, set the bit
            if (color & 0xFFFFFF) {
                p[x/8] |= (1 << (7 - x%8));
            } else {
                p[x/8] &= ~(1 << (7 - x%8));
            }
            break;
    }
}
