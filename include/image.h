#ifndef IMAGE_H
#define IMAGE_H

#include "kernel.h"

#define NUM_CHANNELS 3

struct image;

struct image *image_new(int width, int height, int channels);
struct image *image_load(const char *filename);
struct image *image_apply_kernel(struct image *img, struct kernel *k);
int image_write_pbm(struct image *img, const char *filename);
void image_free(struct image *img);

#endif // IMAGE_H
