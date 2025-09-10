//
// Created by chris on 9/10/25.
//

#include "image_processing.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

/**
 * @brief Checks if a given coordinate is within the image boundaries.
 * @return True if the coordinate is valid, false otherwise.
 */
static bool _is_valid_coordinate(int x, int y) {
    return x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGHT;
}

/**
 * @brief Determines if a single white pixel should be eroded.
 *
 * According to the rules, a white pixel is eroded (turns black) if any of its
 * neighbors defined by the structuring element are black. A neighbor outside
 * the image boundary is treated as black.
 *
 * @param image The binary image buffer.
 * @param x The x-coordinate of the pixel to check.
 * @param y The y-coordinate of the pixel to check.
 * @return True if the pixel should be eroded, false otherwise.
 */
static bool _should_pixel_erode(unsigned char image[BMP_WIDTH][BMP_HEIGHT], const int x, const int y) {
    const int map[3][3] = {
        {0, 1, 0},
        {1, 0, 1},
        {0, 1, 0}
    };

    // Check all neighbors defined by the structuring element
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            // If the element is '1', we need to check this neighbor
            if (map[i + 1][j + 1] == 1) {
                const int neighbor_x = x + i;
                const int neighbor_y = y + j;

                // If the neighbor is outside the image we continue
                if (!_is_valid_coordinate(neighbor_x, neighbor_y)) {
                    continue;
                }

                // If the neighbor is black then erode the pixel
                if (image[neighbor_x][neighbor_y] == 0) {
                    return true; // Erode this pixel
                }
            }
        }
    }

    // If we checked all neighbors and none were black, the pixel survives.
    return false;
}

/**
 * @brief Checks if the exclusion frame is clear of white pixels.
 *
 * @param image The binary image.
 * @param detection_area_size The detection area size (typically 12 -> 12x12)
 * @param exclusion_frame_size The exclusion frame size (typically 1 -> 14x14
 * @param center_x The center x-coordinate of the detection window.
 * @param center_y The center y-coordinate of the detection window.
 * @return True if the frame is all black, false otherwise.
 */
static bool _is_exclusion_frame_clear(unsigned char image[BMP_WIDTH][BMP_HEIGHT], const int detection_area_size,
    const int exclusion_frame_size, const int center_x, const int center_y) {
    assert(detection_area_size % 2 == 0);
    const int half_size = (detection_area_size / 2) + exclusion_frame_size;

    for (int i = -half_size; i < half_size; i++) {
        for (int j = -half_size; j < half_size; j++) {
            // Check if the current pixel is on the frame's border
            if (i == -half_size || i == half_size - 1 || j == -half_size || j == half_size - 1) {
                const int x = center_x + i;
                const int y = center_y + j;

                // If the coordinate is valid AND the pixel is white, the frame is not clear.
                if (_is_valid_coordinate(x, y) && image[x][y] == 255) {
                    return false;
                }
            }
        }
    }
    // If we looped through the whole frame and found no white pixels, it's clear.
    return true;
}

/**
 * @brief Checks if the inner detection area contains at least one white pixel.
 *
 * @param image The binary image.
 * @param detection_area_size The size of the detection area.
 * @param center_x The center x-coordinate of the detection window.
 * @param center_y The center y-coordinate of the detection window.
 * @return True if at least one white pixel is found, false otherwise.
 */
static bool _is_detection_area_active(unsigned char image[BMP_WIDTH][BMP_HEIGHT], const int detection_area_size,
    const int center_x, const int center_y) {
    assert(detection_area_size % 2 == 0); // Dete
    const int half_size = detection_area_size / 2;

    for (int i = -half_size; i < half_size; i++) {
        for (int j = -half_size; j < half_size; j++) {
            const int x = center_x + i;
            const int y = center_y + j;
            if (_is_valid_coordinate(x, y) && image[x][y] == 255) {
                return true; // Found a white pixel!
            }
        }
    }
    return false; // No white pixels found.
}

/**
 * @brief Sets all pixels in the detection area to black.
 *
 * @param image The binary image to modify.
 * @param detection_area_size The detection area size.
 * @param center_x The center x-coordinate of the area to clear.
 * @param center_y The center y-coordinate of the area to clear.
 */
static void _clear_detection_area(unsigned char image[BMP_WIDTH][BMP_HEIGHT], const int detection_area_size,
    const int center_x, const int center_y) {
    assert(detection_area_size % 2 == 0); // Must be a multiple of two
    const int half_size = detection_area_size / 2;
    for (int i = -half_size; i < half_size; i++) {
        for (int j = -half_size; j < half_size; j++) {
            const int x = center_x + i;
            const int y = center_y + j;
            if (_is_valid_coordinate(x, y)) {
                image[x][y] = 0;
            }
        }
    }
}

/**
 * @brief Draws a colored cross marker on the image at a given coordinate.
 *
 * @param image The color image buffer to draw on.
 * @param center_x The center x-coordinate of the cross.
 * @param center_y The center y-coordinate of the cross.
 * @param size The size of the arms of the cross (e.g., size 5 means 11 pixels long).
 * @param r The red component of the marker color (0-255).
 * @param g The green component of the marker color (0-255).
 * @param b The blue component of the marker color (0-255).
 */
static void _draw_cross_marker(unsigned char image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], const int center_x,
    const int center_y, const int size, const unsigned char r, const unsigned char g, const unsigned char b) {
    // Draw the horizontal line of the cross
    for (int i = -size; i <= size; i++) {
        int x = center_x + i;
        if (_is_valid_coordinate(x, center_y)) {
            image[x][center_y][0] = r; // Red channel
            image[x][center_y][1] = g; // Green channel
            image[x][center_y][2] = b; // Blue channel
        }
    }
    // Draw the vertical line of the cross
    for (int j = -size; j <= size; j++) {
        const int y = center_y + j;
        // Avoid re-drawing the center pixel
        if (j != 0 && _is_valid_coordinate(center_x, y)) {
            image[center_x][y][0] = r; // Red channel
            image[center_x][y][1] = g; // Green channel
            image[center_x][y][2] = b; // Blue channel
        }
    }
}

void rgb_to_gray(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS],
    unsigned char output_image[BMP_WIDTH][BMP_HEIGHT]) {
    for (int i = 0; i < BMP_WIDTH; i++) {
        for (int j = 0; j < BMP_HEIGHT; j++) {
            // R: input_image[i][j][0]
            // G: input_image[i][j][1]
            // B: input_image[i][j][2]
            output_image[i][j] = (input_image[i][j][0] + input_image[i][j][1] + input_image[i][j][2]) / 3;
        }
    }
}

void apply_binary_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT],
    unsigned char output_image[BMP_WIDTH][BMP_HEIGHT], const int threshold) {
    for (int i = 0; i < BMP_WIDTH; i++) {
        for (int j = 0; j < BMP_HEIGHT; j++) {
            if (input_image[i][j] > threshold) {
                output_image[i][j] = 255;
            } else {
                output_image[i][j] = 0;
            }
        }
    }
}

int erode_image(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT]) {
    int eroded_pixel_count = 0;

    memcpy(output_image, input_image, BMP_WIDTH * BMP_HEIGHT);

    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGHT; y++) {
            // Check if we are at a white pixel
            if (input_image[x][y] == 255 && _should_pixel_erode(input_image, x, y)) {
                output_image[x][y] = 0; // Erode the pixel in the output buffer
                eroded_pixel_count++;
            }
        }
    }
    return eroded_pixel_count;
}

void detect_spots(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], const int detection_area_size,
    const int exclusion_frame_size, int* cell_count, int cell_coordinates[][2]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGHT; y++) {
            // The exclusion frame must be all black.
            if (_is_exclusion_frame_clear(input_image, detection_area_size, exclusion_frame_size, x, y)) {

                // The inner detection area must contain at least one white pixel.
                if (_is_detection_area_active(input_image, detection_area_size, x, y)) {
                    // Store its coordinates
                    if (*cell_count < 2000) { // Safety check to not overflow the array
                        cell_coordinates[*cell_count][0] = x;
                        cell_coordinates[*cell_count][1] = y;
                        (*cell_count)++;
                    }

                    // Clear the area to prevent detecting the same cell again
                    _clear_detection_area(input_image, detection_area_size, x, y);
                }
            }
        }
    }
}

void generate_output_image(unsigned char original_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], const int cell_count,
    int cell_coordinates[][2]) {
    const int marker_size = 10; // The cross will be 11x11 pixels

    for (int i = 0; i < cell_count; i++) {
        const int x = cell_coordinates[i][0];
        const int y = cell_coordinates[i][1];

        // Draw a red cross (R=255, G=0, B=0) at the cell's location.
        _draw_cross_marker(original_image, x, y, marker_size, 255, 0, 0);
    }
}
