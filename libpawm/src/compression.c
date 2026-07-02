// libpawm/src/compression.c
#include "../include/pawm.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// ========== RLE COMPRESSION ==========
// Simple Run-Length Encoding for byte arrays
// Format: [count] [value] pairs

uint8_t* rle_compress(const uint8_t* input, size_t input_len, size_t* out_len) {
    if (!input || input_len == 0) return NULL;
    
    // Worst case: every byte is different -> 2x size
    uint8_t* output = (uint8_t*)malloc(input_len * 2);
    if (!output) return NULL;
    
    size_t out_idx = 0;
    size_t i = 0;
    
    while (i < input_len) {
        uint8_t val = input[i];
        size_t run_len = 1;
        
        // Count consecutive identical bytes (max 255)
        while (i + run_len < input_len && run_len < 255 && input[i + run_len] == val) {
            run_len++;
        }
        
        output[out_idx++] = (uint8_t)run_len;
        output[out_idx++] = val;
        i += run_len;
    }
    
    *out_len = out_idx;
    return output;
}

uint8_t* rle_decompress(const uint8_t* input, size_t input_len, size_t* out_len) {
    if (!input || input_len == 0) return NULL;
    
    // First pass: calculate output size
    size_t total = 0;
    for (size_t i = 0; i < input_len; i += 2) {
        if (i + 1 >= input_len) break;
        total += input[i];
    }
    
    uint8_t* output = (uint8_t*)malloc(total);
    if (!output) return NULL;
    
    size_t out_idx = 0;
    for (size_t i = 0; i < input_len; i += 2) {
        if (i + 1 >= input_len) break;
        uint8_t count = input[i];
        uint8_t val = input[i + 1];
        memset(output + out_idx, val, count);
        out_idx += count;
    }
    
    *out_len = total;
    return output;
}

// ========== PAWM COMPRESSION WRAPPERS ==========

uint8_t* pawm_compress_pixels(const uint8_t* pixels, size_t pixel_bytes, size_t* compressed_len) {
    return rle_compress(pixels, pixel_bytes, compressed_len);
}

uint8_t* pawm_decompress_pixels(const uint8_t* compressed, size_t compressed_len, size_t* decompressed_len) {
    return rle_decompress(compressed, compressed_len, decompressed_len);
}
