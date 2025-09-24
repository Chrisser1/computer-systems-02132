#include "image_processing.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Checks if a given coordinate is within the image boundaries.
 * @return True if the coordinate is valid, false otherwise.
 */
static bool is_valid_coordinate(const int x, const int y) {
    return x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGHT;
}

void convert_to_grayscale(unsigned char input_image[950][950][3], unsigned char output_image[950][950]) {
    for (int i = 0; i < BMP_WIDTH; i++) {
        for (int j = 0; j < BMP_HEIGHT; j++) {
            // R: input_image[i][j][0]
            // G: input_image[i][j][1]
            // B: input_image[i][j][2]
            output_image[i][j] = (input_image[i][j][0] + input_image[i][j][1] + input_image[i][j][2]) / 3;
        }
    }
}

void convert_to_RGB(unsigned char input_image[950][950], unsigned char output_image[950][950][3]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGHT; y++) {
            output_image[x][y][0] = input_image[x][y];
            output_image[x][y][1] = input_image[x][y];
            output_image[x][y][2] = input_image[x][y];
        }
    }
}

void binary_threshold(unsigned char input_image[950][950], const int threshold) {
    for (int i = 0; i < BMP_WIDTH; i++) {
        for (int j = 0; j < BMP_HEIGHT; j++) {
            if (input_image[i][j] > threshold) {
                input_image[i][j] = 255;
            } else {
                input_image[i][j] = 0;
            }
        }
    }
}

static bool should_pixel_erode(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], const int x, const int y) {
    if (is_valid_coordinate(x-1, y) && input_image[x-1][y] == 0) {
        return true;
    }
    if (is_valid_coordinate(x+1, y) && input_image[x+1][y] == 0) {
        return true;
    }
    if (is_valid_coordinate(x, y-1) && input_image[x][y-1] == 0) {
        return true;
    }
    if (is_valid_coordinate(x, y+1) && input_image[x][y+1] == 0) {
        return true;
    }
    return false;
}

bool erode_image(unsigned char input_image[950][950]) {
    unsigned char output_image[BMP_WIDTH][BMP_HEIGHT];
    // Write everything from the input to the output
    memcpy(output_image, input_image, BMP_WIDTH * BMP_HEIGHT);

    bool has_eroded = false;
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGHT; y++) {
            if (input_image[x][y] == 255 && should_pixel_erode(input_image, x, y)) {
                output_image[x][y] = 0;
                has_eroded = true;
            }
        }
    }
    // Write everything from the output to input
    memcpy(input_image, output_image, BMP_WIDTH * BMP_HEIGHT);
    return has_eroded;
}

Cell_list* create_cell_list() {
    Cell_list* cell_list = malloc(sizeof(Cell_list));

    if (cell_list == NULL) {
        fprintf(stderr, "Failed to allocate cell list\n");
        return NULL;
    }

    cell_list->head = NULL;
    cell_list->cell_amount = 0;
    return cell_list;
}

void add_to_cell_list(Cell_list *list, const int x, const int y) {
    if (list == NULL) {
        fprintf(stderr, "ERROR: The list does not exist.\n");
        return;
    }

    Cell* new_cell = (Cell*)malloc(sizeof(Cell));
    if (new_cell == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for a new cell.\n");
        return;
    }
    new_cell->x = x;
    new_cell->y = y;

    // Assign the current header to the new cell
    new_cell->next = list->head;
    // Set the new cell to be the head of the list
    list->head = new_cell;
    list->cell_amount++;
}

void destroy_cell_list(Cell_list* cell_list) {
    if (cell_list == NULL) {
        return;
    }

    Cell* current = cell_list->head;
    Cell* next;

    // Free all the cells from the list
    while (current) {
        // Save the next cell to next
        next = current->next;
        // Free the current cell
        free(current);
        // Assign the next cell to be the new current
        current = next;
    }

    free(cell_list);
}

bool is_exclusion_frame_clear(unsigned char input_image[950][950], const int detection_area_size,
    const int exclusion_frame_thickness, const int center_x, const int center_y) {

    for (int thickness = 0; thickness <= exclusion_frame_thickness; thickness++) {
        const int half_size = (detection_area_size / 2) + thickness;

        for (int i = -half_size; i < half_size; i++) {
            for (int j = -half_size; j < half_size; j++) {
                // Check if the current pixel is on the frame's border
                if (i == -half_size || i == half_size - 1 || j == -half_size || j == half_size - 1) {
                    const int x = center_x + i;
                    const int y = center_y + j;

                    // If the coordinate is valid AND the pixel is white, the frame is not clear.
                    if (is_valid_coordinate(x, y) && input_image[x][y] == 255) {
                        return false;
                    }
                }
            }
        }
    }
    // If we looped through the whole frame and found no white pixels, it's clear.
    return true;
}

static bool is_detection_area_active(unsigned char image[BMP_WIDTH][BMP_HEIGHT], const int detection_area_size,
    const int center_x, const int center_y) {
    assert(detection_area_size % 2 == 0); // Dete
    const int half_size = detection_area_size / 2;

    for (int i = -half_size; i < half_size; i++) {
        for (int j = -half_size; j < half_size; j++) {
            const int x = center_x + i;
            const int y = center_y + j;
            if (is_valid_coordinate(x, y) && image[x][y] == 255) {
                return true; // Found a white pixel!
            }
        }
    }
    return false; // No white pixels found.
}

static void clear_detection_area(unsigned char image[BMP_WIDTH][BMP_HEIGHT], const int detection_area_size,
    const int center_x, const int center_y) {
    assert(detection_area_size % 2 == 0); // Must be a multiple of two
    const int half_size = detection_area_size / 2;
    for (int i = -half_size; i < half_size; i++) {
        for (int j = -half_size; j < half_size; j++) {
            const int x = center_x + i;
            const int y = center_y + j;
            if (is_valid_coordinate(x, y)) {
                image[x][y] = 0;
            }
        }
    }
}

unsigned int detect_cells(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], const int detection_area_size,
    const int exclusion_frame_thickness, Cell_list *cell_list) {
    unsigned int counter = 0;
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGHT; y++) {
            // The exclusion frame must be all black.
            if (is_exclusion_frame_clear(input_image, detection_area_size, exclusion_frame_thickness, x, y)) {

                // The inner detection area must contain at least one white pixel.
                if (input_image[x][y] || is_detection_area_active(input_image, detection_area_size, x, y)) {
                    // Store its coordinates
                    add_to_cell_list(cell_list, x, y);
                    counter++;

                    // Clear the area to prevent detecting the same cell again
                    clear_detection_area(input_image, detection_area_size, x, y);
                }
            }
        }
    }

    return counter;
}

void draw_points(unsigned char input_image[950][950][3], Cell_list *cell_list) {
    Cell *current = cell_list->head;
    while (current) {
        const int x = current->x;
        const int y = current->y;
        for (int i = -10; i < 10; ++i) {
            // Draw on x-axis
            if (!is_valid_coordinate(x + i, y) && !is_valid_coordinate(x+i, y + 1)
                && !is_valid_coordinate(x + i, y - 1)) {
                continue;
            }
            input_image[x + i][y][0] = 255;
            input_image[x + i][y + 1][0] = 255;
            input_image[x + i][y - 1][0] = 255;
            input_image[x + i][y][1] = 0;
            input_image[x + i][y + 1][1] = 0;
            input_image[x + i][y - 1][1] = 0;
            input_image[x + i][y][2] = 0;
            input_image[x + i][y + 1][2] = 0;
            input_image[x + i][y - 1][2] = 0;

            // Draw on y-axis
            if (!is_valid_coordinate(x, y + i) && !is_valid_coordinate(x + 1, y + i)
                && !is_valid_coordinate(x - 1, y + i)) {
                continue;
            }
            input_image[x][y + i][0] = 255;
            input_image[x + 1][y + i][0] = 255;
            input_image[x - 1][y + i][0] = 255;
            input_image[x][y + i][1] = 0;
            input_image[x + 1][y + i][1] = 0;
            input_image[x - 1][y + i][1] = 0;
            input_image[x][y + i][2] = 0;
            input_image[x + 1][y + i][2] = 0;
            input_image[x - 1][y + i][2] = 0;
        }
        current = current->next;
    }
}
