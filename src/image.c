#include "image.h"
#include "stb_image.h"
#include "util.h"
#include <stdlib.h>

#define NUM_CHANNELS 3

static stbi_uc image_get_pixel(struct image *img, int x, int y, int c);

struct image *image_new(int width, int height, int channels) {
    struct image *img = malloc(sizeof(struct image));
    if (img == NULL) {
        LOG_ERROR("Could not allocate memory for image");
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->channels = channels;
    img->bytes = malloc(width * height * channels * sizeof(stbi_uc));
    if (img->bytes == NULL) {
        LOG_ERROR("Could not allocate memory for image bytes");
        free(img);
        return NULL;
    }

    return img;
}

struct image *image_like(struct image *img) {
    return image_new(img->width, img->height, img->channels);
}

struct image *image_load(const char *filename) {
    struct image *img = malloc(sizeof(struct image));
    if (img == NULL) {
        LOG_ERROR("Could not allocate memory for image");
        return NULL;
    }

    img->bytes = stbi_load(filename, &img->width, &img->height, &img->channels,
                           NUM_CHANNELS);
    if (img->bytes == NULL) {
        LOG_ERROR("Could not load image: %s", filename);
        free(img);
        return NULL;
    }

    return img;
}

struct image *image_apply_kernel(struct image *img, struct kernel *k) {
    return image_apply_kernel_patch(img, k, 0, 0, img->width, img->height);
}

struct image *image_apply_kernel_patch(struct image *img, struct kernel *k,
                                       int start_x, int start_y, int end_x,
                                       int end_y) {
    struct image *new_img =
        image_new(end_x - start_x, end_y - start_y, img->channels);
    if (new_img == NULL) {
        return NULL;
    }

    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            for (int c = 0; c < img->channels; c++) {
                float accum = 0.0f;
                size_t size = k->size;

                for (int ky = 0; ky < size; ky++) {
                    for (int kx = 0; kx < size; kx++) {
                        int img_x = x + kx - size / 2;
                        int img_y = y + ky - size / 2;

                        accum += image_get_pixel(img, img_x, img_y, c) *
                                 kernel_get_value(k, (size - kx - 1),
                                                  (size - ky - 1));
                    }
                }

                if (accum < 0.0f) {
                    accum = 0.0f;
                } else if (accum > 255.0f) {
                    accum = 255.0f;
                }
                int index = (y - start_y) * new_img->width + (x - start_x);
                new_img->bytes[index * new_img->channels + c] = (stbi_uc)accum;
            }
        }
    }

    return new_img;
}

int image_write_pbm(struct image *img, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        LOG_ERROR("Could not open file: %s", filename);
        return 1;
    }

    fprintf(file, "P6\n%d %d\n255\n", img->width, img->height);
    fwrite(img->bytes, sizeof(stbi_uc),
           img->width * img->height * img->channels, file);

    fclose(file);

    return 0;
}

void image_free(struct image *img) {
    stbi_image_free(img->bytes);
    free(img);
}

static stbi_uc image_get_pixel(struct image *img, int x, int y, int c) {
    if (x < 0 || x >= img->width || y < 0 || y >= img->height || c < 0 ||
        c >= img->channels) {
        return 0;
    }

    return img->bytes[(y * img->width + x) * img->channels + c];
}
