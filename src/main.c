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


}