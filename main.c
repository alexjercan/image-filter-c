#include <stdio.h>
#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NUM_CHANNELS 3

#define BLUR_KERNEL_SIZE 3
#define BLUR_KERNEL                                                            \
    {                                                                          \
        1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f, 2.0f / 16.0f, 4.0f / 16.0f,  \
            2.0f / 16.0f, 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f             \
    }

struct kernel {
        int size;
        float *values;
};

struct kernel *new_kernel(const char *name) {
    struct kernel *k = malloc(sizeof(struct kernel));
    if (k == NULL) {
        return NULL;
    }

    if (strcmp(name, "blur") == 0) {
        k->size = BLUR_KERNEL_SIZE;
        k->values = malloc(k->size * k->size * sizeof(float));
        if (k->values == NULL) {
            free(k);
            return NULL;
        }

        float kernel[BLUR_KERNEL_SIZE * BLUR_KERNEL_SIZE] = BLUR_KERNEL;
        memcpy(k->values, kernel, k->size * k->size * sizeof(float));
    } else {
        free(k);
        return NULL;
    }

    return k;
}

void free_kernel(struct kernel *k) {
    free(k->values);
    free(k);
}

struct image {
        int width;
        int height;
        int channels;
        stbi_uc *bytes;
};

struct image *new_image(int width, int height, int channels) {
    struct image *img = malloc(sizeof(struct image));
    if (img == NULL) {
        return NULL;
    }

    img->width = width;
    img->height = height;
    img->channels = channels;
    img->bytes = malloc(width * height * channels * sizeof(stbi_uc));
    if (img->bytes == NULL) {
        free(img);
        return NULL;
    }

    return img;
}

struct image *load_image(const char *filename) {
    struct image *img = malloc(sizeof(struct image));
    if (img == NULL) {
        return NULL;
    }

    img->bytes = stbi_load(filename, &img->width, &img->height, &img->channels,
                           NUM_CHANNELS);
    if (img->bytes == NULL) {
        free(img);
        return NULL;
    }

    return img;
}

void free_image(struct image *img) {
    stbi_image_free(img->bytes);
    free(img);
}

stbi_uc get_pixel(struct image *img, int x, int y, int c) {
    if (x < 0 || x >= img->width || y < 0 || y >= img->height || c < 0 ||
        c >= img->channels) {
        return 0;
    }

    return img->bytes[(y * img->width + x) * img->channels + c];
}

struct image *apply_kernel(struct image *img, struct kernel *k) {
    struct image *new_img = new_image(img->width, img->height, img->channels);

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            for (int c = 0; c < img->channels; c++) {
                float accum = 0.0f;

                for (int ky = 0; ky < k->size; ky++) {
                    for (int kx = 0; kx < k->size; kx++) {
                        int img_x = x + kx - k->size / 2;
                        int img_y = y + ky - k->size / 2;

                        accum += get_pixel(img, img_x, img_y, c) *
                                 k->values[(k->size - ky - 1) * k->size +
                                           (k->size - kx - 1)];
                    }
                }

                new_img->bytes[(y * img->width + x) * img->channels + c] =
                    (stbi_uc)accum;
            }
        }
    }

    return new_img;
}

void save_image_png(struct image *img, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "failed to open file for writing\n");
        return;
    }

    fprintf(file, "P6\n%d %d\n255\n", img->width, img->height);
    fwrite(img->bytes, sizeof(stbi_uc),
           img->width * img->height * img->channels, file);

    fclose(file);
}

int main(int argc, char *argv[]) {
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
    argparse_add_argument(parser, 'f', "filter", "filter name",
                          ARGUMENT_TYPE_VALUE);

    argparse_parse(parser, argc, argv);

    unsigned int help = argparse_get_flag(parser, "help");
    if (help) {
        argparse_print_help(parser);

        argparse_free(parser);
        return 0;
    }

    unsigned int version = argparse_get_flag(parser, "version");
    if (version) {
        argparse_print_version(parser);

        argparse_free(parser);
        return 0;
    }

    char *input = argparse_get_value(parser, "input");
    char *output = argparse_get_value(parser, "output");
    char *filter = argparse_get_value(parser, "filter");

    if (input == NULL || output == NULL || filter == NULL) {
        printf("input, output and filter are required\n");
        argparse_print_help(parser);

        argparse_free(parser);
        return 1;
    }

    struct image *img = load_image(input);
    if (img == NULL) {
        fprintf(stderr, "failed to load image\n");
        argparse_free(parser);
        return 1;
    }

    struct kernel *k = new_kernel(filter);
    struct image *new_img = apply_kernel(img, k);

    save_image_png(new_img, output);

    free_kernel(k);
    free_image(img);
    argparse_free(parser);
    return 0;
}
