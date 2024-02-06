#ifndef KERNEL_H
#define KERNEL_H

#define BLUR_KERNEL_NAME "blur"
#define SHARPEN_KERNEL_NAME "sharpen"
#define EDGE_KERNEL_NAME "edge"
#define EMBOSS_KERNEL_NAME "emboss"

struct kernel {
        int size;
        const float *values;
};

int kernel_from(struct kernel *k, const char *name);
float kernel_get_value_at(struct kernel *k, int x, int y);

#endif // KERNEL_H
