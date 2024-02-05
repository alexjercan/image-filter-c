#include <stdio.h>
#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define return_defer(code)                                                     \
    do {                                                                       \
        result = code;                                                         \
        goto defer;                                                            \
    } while (0)

#define LOG_ERROR(format, ...)                                                 \
    fprintf(stderr, "ERROR: %s:%d: " format "\n", __FILE__, __LINE__,          \
            ##__VA_ARGS__)

#define NUM_CHANNELS 3

#define BLUR_KERNEL_NAME "blur"
#define BLUR_KERNEL_SIZE 3
const float BLUR_KERNEL[] =
    {
        1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f, 2.0f / 16.0f, 4.0f / 16.0f,
            2.0f / 16.0f, 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f
    };

#define SHARPEN_KERNEL_NAME "sharpen"
#define SHARPEN_KERNEL_SIZE 3
const float SHARPEN_KERNEL[] = { 0.0f, -1.0f, 0.0f, -1.0f, 5.0f, -1.0f, 0.0f, -1.0f, 0.0f };

#define EDGE_KERNEL_NAME "edge"
#define EDGE_KERNEL_SIZE 3
const float EDGE_KERNEL[] = { 0.0f, -1.0f, 0.0f, -1.0f, 4.0f, -1.0f, 0.0f, -1.0f, 0.0f };

#define EMBOSS_KERNEL_NAME "emboss"
#define EMBOSS_KERNEL_SIZE 3
const float EMBOSS_KERNEL[] = { -2.0f, -1.0f, 0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 2.0f };

struct kernel {
        int size;
        const float *values;
};

void kernel_init(struct kernel *k, int size, const float *values) {
    k->size = size;
    k->values = values;
}

struct kernel *kernel_new(const char *name) {
    struct kernel *k = malloc(sizeof(struct kernel));
    if (k == NULL) {
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

void kernel_free(struct kernel *k) {
    free(k);
}

struct image {
        int width;
        int height;
        int channels;
        stbi_uc *bytes;
};

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

void image_free(struct image *img) {
    stbi_image_free(img->bytes);
    free(img);
}

stbi_uc image_get_pixel(struct image *img, int x, int y, int c) {
    if (x < 0 || x >= img->width || y < 0 || y >= img->height || c < 0 ||
        c >= img->channels) {
        return 0;
    }

    return img->bytes[(y * img->width + x) * img->channels + c];
}

struct image *image_apply_kernel(struct image *img, struct kernel *k) {
    struct image *new_img = image_new(img->width, img->height, img->channels);
    if (new_img == NULL) {
        return NULL;
    }

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            for (int c = 0; c < img->channels; c++) {
                float accum = 0.0f;

                for (int ky = 0; ky < k->size; ky++) {
                    for (int kx = 0; kx < k->size; kx++) {
                        int img_x = x + kx - k->size / 2;
                        int img_y = y + ky - k->size / 2;

                        accum += image_get_pixel(img, img_x, img_y, c) *
                                 kernel_get_value(k, (k->size - kx - 1),
                                                  (k->size - ky - 1));
                    }
                }

                if (accum < 0.0f) {
                    accum = 0.0f;
                } else if (accum > 255.0f) {
                    accum = 255.0f;
                }
                new_img->bytes[(y * img->width + x) * img->channels + c] =
                    (stbi_uc)accum;
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

int main(int argc, char *argv[]) {
    int result = 0;
    struct image *img = NULL;
    struct kernel *k = NULL;
    struct image *new_img = NULL;

    struct argparse_parser *parser = argparse_new(
        "image filter", "image filter basic implementation", "0.0.1");
    argparse_add_argument(parser, 'v', "version", "print version",
                          ARGUMENT_TYPE_FLAG);
    argparse_add_argument(parser, 'h', "help", "print help",
                          ARGUMENT_TYPE_FLAG);
    argparse_add_argument(parser, 'i', "input", "input file name",
                          ARGUMENT_TYPE_VALUE);
    argparse_add_argument(parser, 'o', "output", "output file name",
                          ARGUMENT_TYPE_VALUE);
    argparse_add_argument(parser, 'f', "filter", "filter name: blur",
                          ARGUMENT_TYPE_VALUE);

    argparse_parse(parser, argc, argv);

    unsigned int help = argparse_get_flag(parser, "help");
    if (help) {
        argparse_print_help(parser);
        return_defer(0);
    }

    unsigned int version = argparse_get_flag(parser, "version");
    if (version) {
        argparse_print_version(parser);
        return_defer(0);
    }

    char *input = argparse_get_value(parser, "input");
    char *output = argparse_get_value(parser, "output");
    char *filter = argparse_get_value(parser, "filter");

    if (input == NULL || output == NULL || filter == NULL) {
        LOG_ERROR("input, output and filter are required");
        argparse_print_help(parser);

        return_defer(1);
    }

    img = image_load(input);
    if (img == NULL) {
        return_defer(1);
    }

    k = kernel_new(filter);
    if (k == NULL) {
        return_defer(1);
    }

    new_img = image_apply_kernel(img, k);
    if (new_img == NULL) {
        return_defer(1);
    }

    if (image_write_pbm(new_img, output) != 0) {
        return_defer(1);
    }

defer:
    if (new_img)
        image_free(new_img);
    if (k)
        kernel_free(k);
    if (img)
        image_free(img);
    if (parser)
        argparse_free(parser);

    return result;
}
