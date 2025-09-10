//
// Created by chris on 9/10/25.
//

#ifndef OS_CHALLENGE_IMAGE_PROCESSING_H
#define OS_CHALLENGE_IMAGE_PROCESSING_H

#include "cbmp.h"

/**
 * @brief Converts rgb image to gray scale.
 *
 * @param input_image The input image as RGB.
 * @param output_image The output of the input image as gray scale.
 */
void rgb_to_gray(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT]);

/**
 * @brief Applies the binary threshold.
 *
 * @param input_image The input image as gray scaled.
 * @param output_image The output image as binary (either 0 or 255).
 * @param threshold The threshold used to decide if the pixel should be black or white.
 */
void apply_binary_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT], int threshold);

/**
 *
 * @param input_image
 * @param output_image
 * @return
 */
int erode_image(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT]);

/**
 *
 * @param input_image
 * @param detection_area_size
 * @param exclusion_frame_size
 * @param cell_count
 * @param cell_coordinates
 */
void detect_spots(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], int detection_area_size, int exclusion_frame_size, int* cell_count, int cell_coordinates[][2]);

/**
 * @brief Makes the output image with a cross at each detected cell
 *
 * @param original_image The original input image.
 * @param cell_count The total amount of cells spotted.
 * @param cell_coordinates The coordinates of the cells.
 */
void generate_output_image(unsigned char original_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int cell_count, int cell_coordinates[][2]);

/**
 * @brief Return the total amount of pixels with the value 255
 *
 * @param image The image to check.
 * @return The total amount of white pixels.
 */
int count_white_pixels(unsigned char image[BMP_WIDTH][BMP_HEIGHT]);

#endif //OS_CHALLENGE_IMAGE_PROCESSING_H