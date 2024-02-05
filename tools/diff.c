#include <stdio.h>
#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "util.h"

#define TOLERANCE 10

int main(int argc, char *argv[]) {
    int result = 0;

    struct argparse_parser *parser = argparse_new(
        "image filter", "image filter basic implementation", "0.0.1");
    argparse_add_argument(parser, 'v', "version", "print version",
                          ARGUMENT_TYPE_FLAG);
    argparse_add_argument(parser, 'h', "help", "print help",
                          ARGUMENT_TYPE_FLAG);
    argparse_add_argument(parser, 'i', "input", "input file name",
                          ARGUMENT_TYPE_VALUE);
    argparse_add_argument(parser, 't', "target", "target file name",
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
    char *target = argparse_get_value(parser, "target");

    if (input == NULL || target == NULL) {
        LOG_ERROR("input and target file names are required");
        return_defer(1);
    }

    int input_width, input_height, input_channels;
    stbi_uc *input_data = stbi_load(input, &input_width, &input_height,
                                     &input_channels, 0);
    if (input_data == NULL) {
        LOG_ERROR("failed to load input image: %s", input);
        return_defer(1);
    }

    int target_width, target_height, target_channels;
    stbi_uc *target_data = stbi_load(target, &target_width, &target_height,
                                     &target_channels, 0);
    if (target_data == NULL) {
        LOG_ERROR("failed to load target image: %s", target);
        return_defer(1);
    }

    if (input_width != target_width || input_height != target_height ||
        input_channels != target_channels) {
        LOG_ERROR("input and target images must have the same dimensions");
        return_defer(1);
    }

    int diff_count = 0;
    for (int i = 0; i < input_width * input_height * input_channels; i++) {
        if (abs(input_data[i] - target_data[i]) > TOLERANCE) {
            diff_count++;
        }
    }

    if (diff_count > 0) {
        LOG_ERROR("images are different: %d pixels differ", diff_count);
        result = 1;
    } else {
        LOG_INFO("images are the same");
        result = 0;
    }

defer:
    if(parser) argparse_free(parser);
    if(input_data) stbi_image_free(input_data);
    if(target_data) stbi_image_free(target_data);
    return result;
}
