#include <pthread.h>
#include <stdio.h>
#include <string.h>
#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"
#include "image.h"
#include "kernel.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "util.h"

int image_apply_kernel_single_thread(struct image *img, struct kernel *k,
                                     struct image *out) {
    return image_apply_kernel(img, k, out);
}

struct thread_args {
        struct image *img;
        struct kernel *k;
        int start_x;
        int start_y;
        int end_x;
        int end_y;
        struct image *out;
};

void *image_apply_kernel_patch_thread(void *args) {
    struct thread_args *a = (struct thread_args *)args;
    return (void *)(long)image_apply_kernel_patch(
        a->img, a->k, a->start_x, a->start_y, a->end_x, a->end_y, a->out);
}

int image_apply_kernel_multi_thread(struct image *img, struct kernel *k,
                                    int threads, struct image *out) {
    pthread_t thread_ids[threads];
    int width = img->width;
    int height = img->height;
    int channels = img->channels;
    int patch_height = height / threads;

    struct thread_args args[threads];
    for (int i = 0; i < threads; i++) {
        int start_y = i * patch_height;
        int end_y = (i + 1) * patch_height;
        if (i == threads - 1) {
            end_y = height;
        }

        args[i].img = img;
        args[i].k = k;
        args[i].start_x = 0;
        args[i].start_y = start_y;
        args[i].end_x = width;
        args[i].end_y = end_y;
        args[i].out = out;

        pthread_create(&thread_ids[i], NULL, image_apply_kernel_patch_thread,
                       args + i);
    }

    int results[threads];
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], (void *)&results[i]);
    }

    for (int i = 0; i < threads; i++) {
        if (results[i] != 0) {
            return 1;
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int result = 0;
    struct image img, out;
    struct kernel k;

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
    argparse_add_argument(parser, 'p', "threads", "number of threads",
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

    int threads = 1;
    char *threads_str = argparse_get_value(parser, "threads");
    if (threads_str) {
        threads = atoi(threads_str);
        if (threads <= 0) {
            LOG_ERROR("threads must be a positive number");
            return_defer(1);
        }
    }

    if (image_load(&img, input) != 0) {
        return_defer(1);
    }

    if (kernel_from(&k, filter) != 0) {
        return_defer(1);
    }

    if (image_init(&out, img.width, img.height, img.channels) != 0) {
        return_defer(1);
    }

    if (threads == 1) {
        image_apply_kernel_single_thread(&img, &k, &out);
    } else {
        image_apply_kernel_multi_thread(&img, &k, threads, &out);
    }

    if (image_write_pbm(&out, output) != 0) {
        return_defer(1);
    }

defer:
    image_destroy(&img);
    image_destroy(&out);
    if (parser)
        argparse_free(parser);

    return result;
}
