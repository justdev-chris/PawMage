// pawm-convert/main.c
#include "../libpawm/include/pawm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// ========== LOAD ANY IMAGE ==========
uint8_t* load_image(const char* path, uint32_t* width, uint32_t* height, uint8_t* depth) {
    int w, h, c;
    uint8_t* data = stbi_load(path, &w, &h, &c, 0);
    if (!data) return NULL;
    *width = w;
    *height = h;
    *depth = c * 8; // 8, 24, or 32
    return data;
}

// ========== SAVE ANY IMAGE ==========
int save_image(const char* path, uint8_t* pixels, uint32_t width, uint32_t height, uint8_t depth) {
    const char* ext = strrchr(path, '.');
    if (!ext) return 0;

    int bpp = depth / 8;
    int stride = width * bpp;

    if (strcmp(ext, ".png") == 0) {
        return stbi_write_png(path, width, height, bpp, pixels, stride);
    }
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
        return stbi_write_jpg(path, width, height, bpp, pixels, 90);
    }
    if (strcmp(ext, ".bmp") == 0) {
        return stbi_write_bmp(path, width, height, bpp, pixels);
    }
    if (strcmp(ext, ".tga") == 0) {
        return stbi_write_tga(path, width, height, bpp, pixels);
    }
    return 0;
}

// ========== USAGE ==========
void print_usage(const char* name) {
    printf("PawMage Converter v1.0\n\n");
    printf("Usage:\n");
    printf("  %s input.(png/jpg/bmp) output.pawm\n", name);
    printf("  %s input.pawm output.(png/jpg/bmp)\n", name);
    printf("  %s --info input.pawm\n", name);
    printf("  %s --set-meta input.pawm key=value output.pawm\n", name);
    printf("\nSupported formats: PNG, JPEG, BMP, TGA\n");
}

// ========== MAIN ==========
int main(int argc, char** argv) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    // --info
    if (strcmp(argv[1], "--info") == 0 && argc == 3) {
        PawMageImage* img = pawm_load(argv[2]);
        if (!img) {
            printf("Failed to load %s\n", argv[2]);
            return 1;
        }
        printf("\n=== PawMage Info ===\n");
        printf("File:    %s\n", argv[2]);
        printf("Width:   %u\n", img->width);
        printf("Height:  %u\n", img->height);
        printf("Depth:   %u bpp\n", img->depth);
        printf("Pixels:  %u bytes\n", img->width * img->height * (img->depth/8));
        printf("Metadata:\n%s\n", img->metadata);
        pawm_free(img);
        return 0;
    }

    // --set-meta
    if (strcmp(argv[1], "--set-meta") == 0 && argc == 5) {
        PawMageImage* img = pawm_load(argv[2]);
        if (!img) {
            printf("Failed to load %s\n", argv[2]);
            return 1;
        }

        char* eq = strchr(argv[3], '=');
        if (!eq) {
            printf("Invalid key=value: %s\n", argv[3]);
            pawm_free(img);
            return 1;
        }
        *eq = '\0';
        char* key = argv[3];
        char* value = eq + 1;

        char* new_meta = pawm_metadata_set(img->metadata, key, value);
        if (new_meta) {
            free(img->metadata);
            img->metadata = new_meta;
            img->metadata_len = strlen(new_meta);
        }

        if (!pawm_save(argv[4], img)) {
            printf("Failed to save %s\n", argv[4]);
            pawm_free(img);
            return 1;
        }

        printf("Updated metadata: %s=%s\n", key, value);
        pawm_free(img);
        return 0;
    }

    // CONVERT
    const char* input = argv[1];
    const char* output = argv[2];

    const char* ext_in = strrchr(input, '.');
    const char* ext_out = strrchr(output, '.');

    if (!ext_in || !ext_out) {
        printf("Missing file extensions\n");
        return 1;
    }

    // ===== TO .pawm =====
    if (strcmp(ext_out, ".pawm") == 0) {
        uint32_t width, height;
        uint8_t depth;
        uint8_t* pixels = load_image(input, &width, &height, &depth);
        if (!pixels) {
            printf("Failed to load %s\n", input);
            return 1;
        }

        PawMageImage img = {
            .width = width,
            .height = height,
            .depth = depth,
            .metadata = "{\"source\":\"pawm-convert\"}",
            .metadata_len = 23,
            .pixels = pixels
        };

        if (!pawm_save(output, &img)) {
            printf("Failed to save %s\n", output);
            stbi_image_free(pixels);
            return 1;
        }

        stbi_image_free(pixels);
        printf("Converted %s -> %s (%ux%u, %u bpp)\n", input, output, width, height, depth);
        return 0;
    }

    // ===== FROM .pawm =====
    if (strcmp(ext_in, ".pawm") == 0) {
        PawMageImage* img = pawm_load(input);
        if (!img) {
            printf("Failed to load %s\n", input);
            return 1;
        }

        if (!save_image(output, img->pixels, img->width, img->height, img->depth)) {
            printf("Failed to save %s (unsupported format?)\n", output);
            pawm_free(img);
            return 1;
        }

        printf("Converted %s -> %s (%ux%u, %u bpp)\n", input, output, img->width, img->height, img->depth);
        pawm_free(img);
        return 0;
    }

    printf("Unsupported input: %s (must be .pawm or .png/.jpg/.bmp/.tga)\n", input);
    return 1;
}
