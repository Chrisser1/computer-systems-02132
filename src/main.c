#include <stdio.h>
#include <string.h>
#include <time.h>

#include "cbmp.h"
#include "image_processing.h"

#define MAX_CELLS 4000
#define FILENAME_BUFFER_SIZE 256

static void construct_output_path(char* output_buffer, size_t buffer_size,
                                  const char* base_path, const char* suffix) {
    const char* extension = strrchr(base_path, '.');
    if (extension != NULL) {
        const int basename_len = extension - base_path;
        snprintf(output_buffer, buffer_size, "%.*s%s%s", basename_len, base_path, suffix, extension);
    } else {
        snprintf(output_buffer, buffer_size, "%s%s", base_path, suffix);
    }
}

unsigned char original_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char rgb_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char grayscale_image[BMP_WIDTH][BMP_HEIGHT];
clock_t start, end;
double cpu_time_used;

int main(int argc, char** argv) {
    // argc counts how may arguments are passed
    // argv[0] is a string with the name of the program
    // argv[1] is the first command line argument (input image)
    // argv[2] is the second command line argument (output image)

    // Check for correct number of arguments
    if (argc != 3) {
        printf("Usage: %s <input_image.bmp> <output_image.bmp>\n", argv[0]);
        return 1;
    }

    // Read the input image from file
    read_bitmap(argv[1], original_image);
    memcpy(rgb_image, original_image, BMP_WIDTH * BMP_HEIGHT * BMP_CHANNELS);

    start = clock();
    convert_to_grayscale(rgb_image, grayscale_image);

    char output_filename[FILENAME_BUFFER_SIZE];

    gaussian_blur_5x5(grayscale_image);
    gaussian_blur_5x5(grayscale_image);
    gaussian_blur_5x5(grayscale_image);
    construct_output_path(output_filename, FILENAME_BUFFER_SIZE, argv[2], "_gaussian");
    convert_to_RGB(grayscale_image, rgb_image);
    write_bitmap(rgb_image, output_filename);

    const int threshold = otsu_threshold_value(grayscale_image);
    printf("The threshold is %i\n", threshold);
    binary_threshold(grayscale_image, threshold);

    construct_output_path(output_filename, FILENAME_BUFFER_SIZE, argv[2], "_binary");
    convert_to_RGB(grayscale_image, rgb_image);
    write_bitmap(rgb_image, output_filename);

    Cell_list* cell_list = create_cell_list();
    int i = 0;
    while (erode_image(grayscale_image)) {
        // detect_cells(grayscale_image, 12, 1, cell_list);
        detect_cells_quick(grayscale_image, cell_list);
        convert_to_RGB(grayscale_image, rgb_image);

        char suffix[32];
        snprintf(suffix, sizeof(suffix), "_erode%d", i);
        construct_output_path(output_filename, FILENAME_BUFFER_SIZE, argv[2], suffix);
        write_bitmap(rgb_image, output_filename);
        i++;
    }
    draw_points(original_image, cell_list);
    printf("Drew %d points \n", cell_list->cell_amount);
    destroy_cell_list(cell_list);
    end = clock();
    cpu_time_used = end - start;
    printf("Time used: %f \n", cpu_time_used);
    write_bitmap(original_image, argv[2]);
}