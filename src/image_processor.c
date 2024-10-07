// src/image_processor.c
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "stb_image.h"
#include "stb_image_write.h"

#define NUM_THREADS 4

typedef enum {
    MODE_GRAYSCALE,
    MODE_RED,
    MODE_GREEN,
    MODE_BLUE
} ColorMode;

typedef struct {
    int thread_id;
    unsigned char *image;
    int width;
    int height;
    int channels;
    int start_row;
    int end_row;
    ColorMode mode;
} thread_data_t;

typedef struct {
    ColorMode mode;
    int upscale;
    int rotate;
} Options;

unsigned char* rotate_image(unsigned char *image, int width, int height, int channels) {
    unsigned char *rotated = malloc(width * height * channels);
    if (!rotated) {
        fprintf(stderr, "Failed to allocate memory for rotated image.\n");
        exit(EXIT_FAILURE);
    }

    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            for(int c = 0; c < channels; c++) {
                rotated[(x * height + (height - y - 1)) * channels + c] = image[(y * width + x) * channels + c];
            }
        }
    }

    return rotated;
}

unsigned char* upscale_image(unsigned char *image, int width, int height, int channels) {
    int new_width = 1920;
    int new_height = 1080;
    unsigned char *upscaled = malloc(new_width * new_height * channels);
    if (!upscaled) {
        fprintf(stderr, "Failed to allocate memory for upscaled image.\n");
        exit(EXIT_FAILURE);
    }

    double x_ratio = (double)width / new_width;
    double y_ratio = (double)height / new_height;
    for(int y = 0; y < new_height; y++) {
        for(int x = 0; x < new_width; x++) {
            int src_x = (int)(x * x_ratio);
            int src_y = (int)(y * y_ratio);
            for(int c = 0; c < channels; c++) {
                upscaled[(y * new_width + x) * channels + c] = image[(src_y * width + src_x) * channels + c];
            }
        }
    }

    free(image);
    return upscaled;
}

// Single-Threaded Processing Function
void process_image_single(unsigned char *image, int width, int height, int channels, ColorMode mode) {
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            int idx = (y * width + x) * channels;
            unsigned char r = image[idx + 0];
            unsigned char g = image[idx + 1];
            unsigned char b = image[idx + 2];

            switch(mode) {
                case MODE_GRAYSCALE: {
                    unsigned char grey = (unsigned char)(0.3 * r + 0.59 * g + 0.11 * b);
                    image[idx + 0] = grey;
                    image[idx + 1] = grey;
                    image[idx + 2] = grey;
                    break;
                }
                case MODE_RED:
                    image[idx + 1] = 0; // Green
                    image[idx + 2] = 0; // Blue
                    break;
                case MODE_GREEN:
                    image[idx + 0] = 0; // Red
                    image[idx + 2] = 0; // Blue
                    break;
                case MODE_BLUE:
                    image[idx + 0] = 0; // Red
                    image[idx + 1] = 0; // Green
                    break;
                default:
                    break;
            }
        }
    }
}

// Multi-Threaded Processing Function (Thread Entry Point)
void* process_image_multi(void *arg) {
    thread_data_t *data = (thread_data_t*) arg;
    unsigned char *img = data->image;
    int width = data->width;
    int channels = data->channels;
    int start = data->start_row;
    int end = data->end_row;
    ColorMode mode = data->mode;

    printf("Thread %d processing rows %d to %d\n", data->thread_id, start, end - 1);

    for(int y = start; y < end; y++) {
        for(int x = 0; x < width; x++) {
            int idx = (y * width + x) * channels;
            unsigned char r = img[idx + 0];
            unsigned char g = img[idx + 1];
            unsigned char b = img[idx + 2];

            switch(mode) {
                case MODE_GRAYSCALE: {
                    unsigned char grey = (unsigned char)(0.3 * r + 0.59 * g + 0.11 * b);
                    img[idx + 0] = grey;
                    img[idx + 1] = grey;
                    img[idx + 2] = grey;
                    break;
                }
                case MODE_RED:
                    img[idx + 1] = 0; // Green
                    img[idx + 2] = 0; // Blue
                    break;
                case MODE_GREEN:
                    img[idx + 0] = 0; // Red
                    img[idx + 2] = 0; // Blue
                    break;
                case MODE_BLUE:
                    img[idx + 0] = 0; // Red
                    img[idx + 1] = 0; // Green
                    break;
                default:
                    break;
            }
        }
    }

    printf("Thread %d completed processing.\n", data->thread_id);
    pthread_exit(NULL);
}

int main() {
    Options options;
    char image_path[256];
    clock_t start_time, end_time;
    double time_single, time_multi;

    printf("Single and Multi-Threaded Image Processor\n");
    printf("\n");

    // Get image path
    printf("Enter the path to the input image: ");
    if (scanf("%s", image_path) != 1) {
        fprintf(stderr, "Error reading input.\n");
        return EXIT_FAILURE;
    }

    // Get color mode
    printf("\nSelect Color Mode:\n");
    printf("1. Grayscale\n");
    printf("2. Red\n");
    printf("3. Green\n");
    printf("4. Blue\n");
    printf("Enter choice (1-4): ");
    int mode_choice;
    if (scanf("%d", &mode_choice) != 1) {
        fprintf(stderr, "Invalid input.\n");
        return EXIT_FAILURE;
    }

    switch(mode_choice) {
        case 1:
            options.mode = MODE_GRAYSCALE;
            break;
        case 2:
            options.mode = MODE_RED;
            break;
        case 3:
            options.mode = MODE_GREEN;
            break;
        case 4:
            options.mode = MODE_BLUE;
            break;
        default:
            printf("Invalid choice. Defaulting to Grayscale.\n");
            options.mode = MODE_GRAYSCALE;
            break;
    }

    // Get upscaling option
    printf("\nDo you want to upscale the image to 1080p (1920x1080)? (y/n): ");
    char upscale_choice;
    if (scanf(" %c", &upscale_choice) != 1) {
        fprintf(stderr, "Invalid input.\n");
        return EXIT_FAILURE;
    }
    options.upscale = (upscale_choice == 'y' || upscale_choice == 'Y') ? 1 : 0;

    // Get rotation option
    printf("Do you want to rotate the image by 90 degrees? (y/n): ");
    char rotate_choice;
    if (scanf(" %c", &rotate_choice) != 1) {
        fprintf(stderr, "Invalid input.\n");
        return EXIT_FAILURE;
    }
    options.rotate = (rotate_choice == 'y' || rotate_choice == 'Y') ? 1 : 0;

    // Load image
    int width, height, channels;
    unsigned char *image = stbi_load(image_path, &width, &height, &channels, 3); // Force 3 channels
    if(image == NULL) {
        fprintf(stderr, "Error: Could not load image %s\n", image_path);
        return EXIT_FAILURE;
    }

    // Rotate image if requested
    if(options.rotate) {
        unsigned char *rotated = rotate_image(image, width, height, 3);
        // Update width and height
        int temp = width;
        width = height;
        height = temp;
        image = rotated;
    }

    // Upscale image if requested
    if(options.upscale) {
        image = upscale_image(image, width, height, 3);
        width = 1920;
        height = 1080;
    }

    // =================== Single-Threaded Processing ===================
    printf("\nStarting Single-Threaded Processing...\n");
    start_time = clock();
    process_image_single(image, width, height, 3, options.mode);
    end_time = clock();
    time_single = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Single-Threaded Processing Time: %.6f seconds\n", time_single);

    // Save single-threaded output
    char output_single[256] = "output_single.jpg";
    if(stbi_write_jpg(output_single, width, height, 3, image, 100)) {
        printf("Single-Threaded Output saved as %s\n", output_single);
    }
    else {
        fprintf(stderr, "Error: Could not save image %s\n", output_single);
    }

    // =================== Multi-Threaded Processing ===================
    printf("\nReloading image for Multi-Threaded Processing...\n");
    stbi_image_free(image);
    image = stbi_load(image_path, &width, &height, &channels, 3); // Force 3 channels
    if(image == NULL) {
        fprintf(stderr, "Error: Could not reload image %s\n", image_path);
        return EXIT_FAILURE;
    }

    // Rotate image if requested
    if(options.rotate) {
        unsigned char *rotated = rotate_image(image, width, height, 3);
        // Update width and height
        int temp = width;
        width = height;
        height = temp;
        image = rotated;
    }

    // Upscale image if requested
    if(options.upscale) {
        image = upscale_image(image, width, height, 3);
        width = 1920;
        height = 1080;
    }

    printf("\nStarting Multi-Threaded Processing...\n");
    clock_t mt_start = clock();

    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data_array[NUM_THREADS];
    int rows_per_thread = height / NUM_THREADS;

    for(int i = 0; i < NUM_THREADS; i++) {
        thread_data_array[i].thread_id = i;
        thread_data_array[i].image = image;
        thread_data_array[i].width = width;
        thread_data_array[i].height = height;
        thread_data_array[i].channels = 3;
        thread_data_array[i].start_row = i * rows_per_thread;
        thread_data_array[i].end_row = (i == NUM_THREADS - 1) ? height : (i + 1) * rows_per_thread;
        thread_data_array[i].mode = options.mode;
        int rc = pthread_create(&threads[i], NULL, process_image_multi, (void*)&thread_data_array[i]);
        if(rc) {
            fprintf(stderr, "Error: Unable to create thread %d\n", i);
            stbi_image_free(image);
            return EXIT_FAILURE;
        }
    }

    // Wait for all threads to complete
    for(int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t mt_end = clock();
    time_multi = ((double)(mt_end - mt_start)) / CLOCKS_PER_SEC;
    printf("Multi-Threaded Processing Time: %.6f seconds\n", time_multi);

    // Save multi-threaded output
    char output_multi[256] = "output_multi.jpg";
    if(stbi_write_jpg(output_multi, width, height, 3, image, 100)) {
        printf("Multi-Threaded Output saved as %s\n", output_multi);
    }
    else {
        fprintf(stderr, "Error: Could not save image %s\n", output_multi);
    }

    // =================== Performance Comparison ===================
    printf("\nPerformance Comparison:\n");
    printf("\n");
    printf("Single-Threaded: %.6f seconds\n", time_single);
    printf("Multi-Threaded:  %.6f seconds\n", time_multi);

    // Simple ASCII bar graph
    printf("\nExecution Time Comparison:\n");
    printf("\n");
    int max_length = 50;
    double max_time = (time_single > time_multi) ? time_single : time_multi;
    int single_length = (int)((time_single / max_time) * max_length);
    int multi_length = (int)((time_multi / max_time) * max_length);

    printf("Single-Threaded: [");
    for(int i = 0; i < single_length; i++) printf("#");
    for(int i = single_length; i < max_length; i++) printf(" ");
    printf("] %.6f s\n", time_single);

    printf("Multi-Threaded:  [");
    for(int i = 0; i < multi_length; i++) printf("#");
    for(int i = multi_length; i < max_length; i++) printf(" ");
    printf("] %.6f s\n", time_multi);

    // Conclusion
    printf("\nConclusion:\n");
    if(time_multi < time_single) {
        double improvement = ((time_single - time_multi) / time_single) * 100;
        printf("Multi-threaded processing is faster by %.2f%% compared to single-threaded processing.\n", improvement);
    }
    else {
        double improvement = ((time_multi - time_single) / time_multi) * 100;
        printf("Single-threaded processing is faster by %.2f%% compared to multi-threaded processing.\n", improvement);
    }
    printf("This demonstrates the advantages of multi-threading in operating systems for parallelizing tasks and improving performance.\n");

    // Free the image memory
    stbi_image_free(image);

    return EXIT_SUCCESS;
}
