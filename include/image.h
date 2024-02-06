#ifndef IMAGE_H
#define IMAGE_H

#include "kernel.h"

#define NUM_CHANNELS 3

struct image {
        int width;
        int height;
        int channels;
        unsigned char *bytes;
};

int image_init(struct image *img, int width, int height, int channels);
int image_load(struct image *img, const char *filename);
int image_apply_kernel(struct image *img, struct kernel *k, struct image *out);
int image_apply_kernel_patch(struct image *img, struct kernel *k, int start_x,
                             int start_y, int end_x, int end_y,
                             struct image *out);
int image_write_pbm(struct image *img, const char *filename);
void image_destroy(struct image *img);

#endif // IMAGE_H
