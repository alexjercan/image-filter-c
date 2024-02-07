#include "util.h"

#define NUM_THREADS_PER_BLOCK 32

__global__ void applyKernel(unsigned char *d_img_bytes, int width, int height,
                            int channels, float *d_kernel, int size,
                            unsigned char *d_out_bytes) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int c = blockIdx.z * blockDim.z + threadIdx.z;

    if (x >= width || y >= height || c >= channels)
        return;

    float accum = 0.0f;

    for (int ky = 0; ky < size; ky++) {
        for (int kx = 0; kx < size; kx++) {
            int img_x = x + kx - size / 2;
            int img_y = y + ky - size / 2;
            int k_x = size - kx - 1;
            int k_y = size - ky - 1;

            unsigned char pixel = 0;
            if (img_x >= 0 && img_x < width && img_y >= 0 && img_y < height &&
                c >= 0 && c < channels) {
                pixel = d_img_bytes[(img_y * width + img_x) * channels + c];
            }

            float value = 0.0f;
            if (k_x >= 0 && k_x < size && k_y >= 0 && k_y < size) {
                value = d_kernel[k_y * size + k_x];
            }

            accum += pixel * value;
        }
    }

    if (accum < 0.0f) {
        accum = 0.0f;
    } else if (accum > 255.0f) {
        accum = 255.0f;
    }
    d_out_bytes[(y * width + x) * channels + c] = (unsigned char)accum;
}

static int imageApplyKernel(unsigned char *d_img_bytes, int width, int height,
                            int channels, float *d_kernel, int size,
                            unsigned char *d_out_bytes) {
    dim3 threadsPerBlock(NUM_THREADS_PER_BLOCK, NUM_THREADS_PER_BLOCK, 1);
    dim3 numBlocks((width + threadsPerBlock.x - 1) / threadsPerBlock.x,
                   (height + threadsPerBlock.y - 1) / threadsPerBlock.y,
                   (channels + threadsPerBlock.z - 1) / threadsPerBlock.z);

    applyKernel<<<numBlocks, threadsPerBlock>>>(
        d_img_bytes, width, height, channels, d_kernel, size, d_out_bytes);
    cudaDeviceSynchronize();

    return 0;
}

extern "C" {
#include "image.h"

int image_apply_kernel_cuda_wrapper(struct image *img, struct kernel *k,
                                    struct image *out, int repeats) {
    int result = 0;
    unsigned char *d_img_bytes = NULL;
    unsigned char *d_out_bytes = NULL;
    float *d_kernel = NULL;

    cudaError_t error;

    error = cudaMalloc((void **)&d_img_bytes, img->width * img->height *
                                                  img->channels *
                                                  sizeof(unsigned char));
    if (error != cudaSuccess) {
        LOG_ERROR("cudaMalloc failed: %s\n", cudaGetErrorString(error));
        return_defer(1);
    }

    error = cudaMalloc((void **)&d_out_bytes, img->width * img->height *
                                                  img->channels *
                                                  sizeof(unsigned char));
    if (error != cudaSuccess) {
        LOG_ERROR("cudaMalloc failed: %s\n", cudaGetErrorString(error));
        return_defer(1);
    }

    error = cudaMalloc((void **)&d_kernel, k->size * k->size * sizeof(float));
    if (error != cudaSuccess) {
        LOG_ERROR("cudaMalloc failed: %s\n", cudaGetErrorString(error));
        return_defer(1);
    }

    error = cudaMemcpy(d_img_bytes, img->bytes,
                       img->width * img->height * img->channels *
                           sizeof(unsigned char),
                       cudaMemcpyHostToDevice);
    if (error != cudaSuccess) {
        LOG_ERROR("cudaMalloc failed: %s\n", cudaGetErrorString(error));
        return_defer(1);
    }

    error = cudaMemcpy(d_kernel, k->values, k->size * k->size * sizeof(float),
                       cudaMemcpyHostToDevice);
    if (error != cudaSuccess) {
        LOG_ERROR("cudaMalloc failed: %s\n", cudaGetErrorString(error));
        return_defer(1);
    }

    for (int i = 0; i < repeats; i++) {
        imageApplyKernel(d_img_bytes, img->width, img->height, img->channels,
                         d_kernel, k->size, d_out_bytes);
        cudaMemcpy(d_img_bytes, d_out_bytes,
                   img->width * img->height * img->channels *
                       sizeof(unsigned char),
                   cudaMemcpyDeviceToDevice);
    }

    error = cudaMemcpy(out->bytes, d_out_bytes,
                       img->width * img->height * img->channels *
                           sizeof(unsigned char),
                       cudaMemcpyDeviceToHost);
    if (error != cudaSuccess) {
        LOG_ERROR("cudaMalloc failed: %s\n", cudaGetErrorString(error));
        return_defer(1);
    }

defer:
    if (d_img_bytes)
        cudaFree(d_img_bytes);
    if (d_out_bytes)
        cudaFree(d_out_bytes);

    return result;
}
}
