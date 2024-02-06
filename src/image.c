#include "image.h"
#include "stb_image.h"
#include "util.h"
#include <stdlib.h>

#define NUM_CHANNELS 3

static stbi_uc image_get_pixel(struct image *img, int x, int y, int c);

int image_init(struct image *img, int width, int height, int channels) {
    img->width = width;
    img->height = height;
    img->channels = channels;
    img->bytes = malloc(width * height * channels * sizeof(stbi_uc));

    if (img->bytes == NULL) {
        LOG_ERROR("Could not allocate memory for image bytes");
        return 1;
    }

    return 0;
}

int image_load(struct image *img, const char *filename) {
    img->bytes = stbi_load(filename, &img->width, &img->height, &img->channels,
                           NUM_CHANNELS);
    if (img->bytes == NULL) {
        LOG_ERROR("Could not load image: %s", filename);
        return 1;
    }

    return 0;
}

int image_apply_kernel(struct image *img, struct kernel *k, struct image *out) {
    return image_apply_kernel_patch(img, k, 0, 0, img->width, img->height, out);
}

int image_apply_kernel_patch(struct image *img, struct kernel *k, int start_x,
                             int start_y, int end_x, int end_y,
                             struct image *out) {
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            for (int c = 0; c < img->channels; c++) {
                float accum = 0.0f;
                size_t size = k->size;

                for (int ky = 0; ky < size; ky++) {
                    for (int kx = 0; kx < size; kx++) {
                        int img_x = x + kx - size / 2;
                        int img_y = y + ky - size / 2;
                        int k_x = size - kx - 1;
                        int k_y = size - ky - 1;

                        stbi_uc pixel = image_get_pixel(img, img_x, img_y, c);
                        float value = kernel_get_value_at(k, k_x, k_y);
                        accum += pixel * value;
                    }
                }

                if (accum < 0.0f) {
                    accum = 0.0f;
                } else if (accum > 255.0f) {
                    accum = 255.0f;
                }
                int index = y * out->width + x;
                out->bytes[index * out->channels + c] = (stbi_uc)accum;
            }
        }
    }

    return 0;
}

int image_write_pbm(struct image *img, const char *filename) {
    FILE *file = fopen(filename, "wb");
    int result = 0;
    if (file == NULL) {
        LOG_ERROR("Could not open file: %s", filename);
        return_defer(1);
    }

    if (img->channels != 3) {
        LOG_ERROR("Image must have 3 channels to write to PBM");
        return_defer(1);
    }

    if (img->bytes == NULL) {
        LOG_ERROR("Image has no bytes to write to PBM");
        return_defer(1);
    }

    fprintf(file, "P6\n%d %d\n255\n", img->width, img->height);
    fwrite(img->bytes, sizeof(stbi_uc),
           img->width * img->height * img->channels, file);

defer:
    if (file)
        fclose(file);

    return 0;
}

void image_destroy(struct image *img) { stbi_image_free(img->bytes); }

static stbi_uc image_get_pixel(struct image *img, int x, int y, int c) {
    if (x < 0 || x >= img->width || y < 0 || y >= img->height || c < 0 ||
        c >= img->channels) {
        return 0;
    }

    return img->bytes[(y * img->width + x) * img->channels + c];
}
