/**
* @file main.c
 * @brief Main file for the cell detection program.
 *
 * This file contains the main function that drives the cell detection algorithm.
 * It handles command-line arguments for input and output image paths,
 * calls the various image processing steps, and prints the final results.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cbmp.h"
#include "image_processing.h"

#define MAX_CELLS 4000

int main(int argc, char** argv) {
    //argc counts how may arguments are passed
    //argv[0] is a string with the name of the program
    //argv[1] is the first command line argument (input image)
    //argv[2] is the second command line argument (output image)

    // Check for correct number of arguments
    if (argc != 3) {
        printf("Usage: %s <input_image.bmp> <output_image.bmp>\n", argv[0]);
        return 1;
    }

    // Allocate memory for the images
    unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
    unsigned char gray_image[BMP_WIDTH][BMP_HEIGHT];

    // Variables for storing detection results
    int cell_count = 0;
    int cell_coordinates[MAX_CELLS][2];

    // Read the input image from file
    read_bitmap(argv[1], input_image);

    // Convert to gray-scale
    rgb_to_gray(input_image, gray_image);

    // Apply binary threshold to get the initial binary image
    unsigned char current_binary_image[BMP_WIDTH][BMP_HEIGHT];
    unsigned char next_binary_image[BMP_WIDTH][BMP_HEIGHT];
    apply_binary_threshold(gray_image, current_binary_image, 90);

    int eroded_pixels_last_pass = 1;
    while (eroded_pixels_last_pass > 0) {
        // First, detect spots on the current image. This happens *before* erosion.
        detect_spots(current_binary_image, 12, 1, &cell_count, cell_coordinates);

        // Then, erode the current image and store the result in the next buffer
        eroded_pixels_last_pass = erode_image(current_binary_image, next_binary_image);

        // Swap the buffers for the next iteration by copying the memory
        memcpy(current_binary_image, next_binary_image, BMP_WIDTH * BMP_HEIGHT);
    }

    // Create the final output image by drawing markers on the original input image
    generate_output_image(input_image, cell_count, cell_coordinates);

    // Save the marked-up image to the output file
    write_bitmap(input_image, argv[2]);

    // Print the results to the console as required [cite: 823]
    printf("Cell detection complete.\n");
    printf("Total cells detected: %d\n", cell_count);
    return 0;
}