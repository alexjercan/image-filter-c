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

struct image *image_new(int width, int height, int channels);
struct image *image_like(struct image *img);
struct image *image_load(const char *filename);
struct image *image_apply_kernel(struct image *img, struct kernel *k);
struct image *image_apply_kernel_patch(struct image *img, struct kernel *k,
                                       int start_x, int start_y, int end_x,
                                       int end_y);
int image_write_pbm(struct image *img, const char *filename);
void image_free(struct image *img);

#endif // IMAGE_H
