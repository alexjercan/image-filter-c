#ifndef KERNEL_H
#define KERNEL_H

#define BLUR_KERNEL_NAME "blur"
#define SHARPEN_KERNEL_NAME "sharpen"
#define EDGE_KERNEL_NAME "edge"
#define EMBOSS_KERNEL_NAME "emboss"

struct kernel;

struct kernel *kernel_new(const char *name);
float kernel_get_value(struct kernel *k, int x, int y);
int kernel_get_size(struct kernel *k);
void kernel_free(struct kernel *k);

#endif // KERNEL_H
