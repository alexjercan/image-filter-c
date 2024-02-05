#include "kernel.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

const int BLUR_KERNEL_SIZE = 3;
const float BLUR_KERNEL[] = {1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
                             2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
                             1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f};

const int SHARPEN_KERNEL_SIZE = 3;
const float SHARPEN_KERNEL[] = {0.0f,  -1.0f, 0.0f,  -1.0f, 5.0f,
                                -1.0f, 0.0f,  -1.0f, 0.0f};

const int EDGE_KERNEL_SIZE = 3;
const float EDGE_KERNEL[] = {0.0f,  -1.0f, 0.0f,  -1.0f, 4.0f,
                             -1.0f, 0.0f,  -1.0f, 0.0f};

const int EMBOSS_KERNEL_SIZE = 3;
const float EMBOSS_KERNEL[] = {-2.0f, -1.0f, 0.0f, -1.0f, 1.0f,
                               1.0f,  0.0f,  1.0f, 2.0f};

struct kernel {
        int size;
        const float *values;
};

static void kernel_init(struct kernel *k, int size, const float *values) {
    k->size = size;
    k->values = values;
}

struct kernel *kernel_new(const char *name) {
    struct kernel *k = malloc(sizeof(struct kernel));
    if (k == NULL) {
        LOG_ERROR("Could not allocate memory for kernel");
        return NULL;
    }

    if (strcmp(name, BLUR_KERNEL_NAME) == 0) {
        kernel_init(k, BLUR_KERNEL_SIZE, BLUR_KERNEL);
    } else if (strcmp(name, SHARPEN_KERNEL_NAME) == 0) {
        kernel_init(k, SHARPEN_KERNEL_SIZE, SHARPEN_KERNEL);
    } else if (strcmp(name, EDGE_KERNEL_NAME) == 0) {
        kernel_init(k, EDGE_KERNEL_SIZE, EDGE_KERNEL);
    } else if (strcmp(name, EMBOSS_KERNEL_NAME) == 0) {
        kernel_init(k, EMBOSS_KERNEL_SIZE, EMBOSS_KERNEL);
    } else {
        LOG_ERROR("Unknown kernel name: %s", name);
        free(k);
        return NULL;
    }

    return k;
}

float kernel_get_value(struct kernel *k, int x, int y) {
    if (x < 0 || x >= k->size || y < 0 || y >= k->size) {
        return 0.0f;
    }

    return k->values[y * k->size + x];
}

int kernel_get_size(struct kernel *k) { return k->size; }

void kernel_free(struct kernel *k) { free(k); }
