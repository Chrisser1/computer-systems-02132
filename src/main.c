/**
* @file main.c
 * @brief Main file for the cell detection program.
 *
 * This file contains the main function that drives the cell detection algorithm.
 * It handles command-line arguments for input and output image paths,
 * calls the various image processing steps, and prints the final results.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "cbmp.h"
#include "image_processing.h"

#define MAX_CELLS 4000

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
    read_bitmap(argv[1], rgb_image);

    start = clock();
    convert_to_grayscale(rgb_image, grayscale_image);
    const int threshold = otsu_threshold_value(grayscale_image);
    binary_threshold(grayscale_image, threshold);
    convert_to_RGB(grayscale_image, rgb_image);
    write_bitmap(rgb_image, argv[2]);

    unsigned int counter = 0;
    Cell_list* cell_list = create_cell_list();
    while (erode_image(grayscale_image)) {
        counter += detect_cells(grayscale_image, 12, 1, cell_list);
    }
    printf("Amount of times eroded: %d\n", counter);

    draw_points(rgb_image, cell_list);
    printf("Drew %d points \n", cell_list->cell_amount);
    destroy_cell_list(cell_list);
    end = clock();
    cpu_time_used = end - start;
    printf("Time used: %f \n", cpu_time_used);
    write_bitmap(rgb_image, argv[2]);
}