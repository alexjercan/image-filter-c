#include <stdio.h>
#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"
#include "image.h"
#include "kernel.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "util.h"

struct image *image_apply_kernel_single_thread(struct image *img,
                                               struct kernel *k) {
    return image_apply_kernel(img, k);
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

    img = image_load(input);
    if (img == NULL) {
        return_defer(1);
    }

    k = kernel_new(filter);
    if (k == NULL) {
        return_defer(1);
    }

    if (threads == 1) {
        new_img = image_apply_kernel_single_thread(img, k);
    } else {
        LOG_ERROR("multi-threading is not supported yet");
        return_defer(1);
    }

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
