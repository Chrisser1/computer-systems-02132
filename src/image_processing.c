#include "image_processing.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef BORDER
#define BORDER 1
#endif

/**
 * @brief Checks if a given coordinate is within the image boundaries.
 * @return True if the coordinate is valid, false otherwise.
 */
static bool is_valid_coordinate(const int x, const int y) {
    return x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGHT;
}

void convert_to_grayscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT]) {
    for (int i = 0; i < BMP_WIDTH; i++) {
        for (int j = 0; j < BMP_HEIGHT; j++) {
            // R: input_image[i][j][0]
            // G: input_image[i][j][1]
            // B: input_image[i][j][2]
            output_image[i][j] = (input_image[i][j][0] + input_image[i][j][1] + input_image[i][j][2]) / 3;
        }
    }
}

void convert_to_RGB(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGHT; y++) {
            output_image[x][y][0] = input_image[x][y];
            output_image[x][y][1] = input_image[x][y];
            output_image[x][y][2] = input_image[x][y];
        }
    }
}

void apply_convolution(unsigned char image[BMP_WIDTH][BMP_HEIGHT],
                       const int* kernel, const int kernel_size) {
    // A convolution kernel must have an odd size
    if (kernel_size % 2 == 0) {
        printf("Error: Kernel size must be odd.\n");
        return;
    }

    // Calculate the radius from the kernel size
    const int radius = kernel_size / 2;

    // Calculate the divisor by summing all kernel elements
    int divisor = 0;
    for (int i = 0; i < kernel_size * kernel_size; ++i) {
        divisor += kernel[i];
    }
    // Prevent division by zero
    if (divisor == 0) {
        divisor = 1;
    }

    unsigned char output_image[BMP_WIDTH][BMP_HEIGHT];

    // Main loops now use the radius for border handling
    for (int x = radius; x < BMP_WIDTH - radius; x++) {
        for (int y = radius; y < BMP_HEIGHT - radius; y++) {
            int sum = 0;

            // Kernel loops also use the radius
            for (int i = -radius; i <= radius; i++) {
                for (int j = -radius; j <= radius; j++) {
                    // Calculate the 1D index for the flattened kernel array
                    const int kernel_row = i + radius;
                    const int kernel_col = j + radius;
                    const int kernel_index = kernel_row * kernel_size + kernel_col;

                    sum += image[x + i][y + j] * kernel[kernel_index];
                }
            }
            output_image[x][y] = (unsigned char)(sum / divisor);
        }
    }

    // Copy the processed inner pixels back to the original image
    for (int x = radius; x < BMP_WIDTH - radius; x++) {
        for (int y = radius; y < BMP_HEIGHT - radius; y++) {
            image[x][y] = output_image[x][y];
        }
    }
}

void gaussian_blur_3x3(unsigned char image[BMP_WIDTH][BMP_HEIGHT]) {
    const int kernel[] = {
        1, 2, 1,
        2, 4, 2,
        1, 2, 1
    };
    apply_convolution(image, kernel, 3);
}

void gaussian_blur_5x5(unsigned char image[BMP_WIDTH][BMP_HEIGHT]) {
    const int kernel[] = {
        1,  4,  7,  4, 1,
        4, 16, 26, 16, 4,
        7, 26, 41, 26, 7,
        4, 16, 26, 16, 4,
        1,  4,  7,  4, 1
    };
    apply_convolution(image, kernel, 5);
}

void sharpen_image(unsigned char image[BMP_WIDTH][BMP_HEIGHT]) {
    const int kernel[] = {
        0, -1,  0,
       -1,  5, -1,
        0, -1,  0
   };
    apply_convolution(image, kernel, 3);
}

unsigned char otsu_threshold_value(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT]) {
    // Init the histogram
    int histogram[256];
    for (int i = 0; i < 256; ++i) {
        histogram[i] = 0;
    }
    for (int x = 0; x < BMP_WIDTH; ++x) {
        for (int y = 0; y < BMP_HEIGHT; ++y) {
            histogram[input_image[x][y]]++;
        }
    }

    // Iterate over histogram and add the pixel values
    double best_otsu = 0;
    int best_split = 0;
    const double total_pixels = BMP_WIDTH * BMP_HEIGHT;
    for (int split = 0; split < 256; ++split) {
        int b_sum = 0;
        int mu_b_sum = 0;
        int f_sum = 0;
        int mu_f_sum = 0;

        for (int value = 0; value < 256; ++value) {
            if (value <= split) {
                b_sum += histogram[value];
                mu_b_sum += histogram[value] * value;
            } else {
                f_sum += histogram[value];
                mu_f_sum += histogram[value] * value;
            }
        }

        const double W_b = (double)b_sum / total_pixels;
        const double W_f = (double)f_sum / total_pixels;

        if (b_sum == 0 || f_sum == 0) {
            continue;
        }
        const double mu_b = (double)mu_b_sum / b_sum;;
        const double mu_f = (double)mu_f_sum / f_sum;

        const double otsu = W_b * W_f * (double)((mu_b - mu_f) * (mu_b - mu_f));

        if (best_otsu < otsu) {
            best_otsu = otsu;
            best_split = split;
        }
    }

    return best_split;
}

void binary_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], const int threshold) {
    for (int x = 0; x < BMP_WIDTH; ++x) {
        for (int y = 0; y < BMP_HEIGHT; ++y) {
            input_image[x][y] = (input_image[x][y] > threshold) ? 255 : 0;
        }
    }

    if (BORDER > 0) {
        const int bw = (BORDER < BMP_WIDTH)  ? BORDER : BMP_WIDTH;
        const int bh = (BORDER < BMP_HEIGHT) ? BORDER : BMP_HEIGHT;

        // top & bottom rows
        for (int x = 0; x < BMP_WIDTH; ++x) {
            for (int y = 0; y < bh; ++y) {
                input_image[x][y] = 0;
                input_image[x][BMP_HEIGHT - 1 - y] = 0;
            }
        }
        // left & right columns
        for (int y = 0; y < BMP_HEIGHT; ++y) {
            for (int x = 0; x < bw; ++x) {
                input_image[x][y] = 0;
                input_image[BMP_WIDTH - 1 - x][y] = 0;
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

bool erode_image(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT]) {
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

    Cell* new_cell = malloc(sizeof(Cell));
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

bool is_exclusion_frame_clear(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], const int detection_area_size,
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

void detect_cells(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], const int detection_area_size,
    const int exclusion_frame_thickness, Cell_list *cell_list) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGHT; y++) {
            // The exclusion frame must be all black.
            if (is_exclusion_frame_clear(input_image, detection_area_size, exclusion_frame_thickness, x, y)) {

                // The inner detection area must contain at least one white pixel.
                if (input_image[x][y] || is_detection_area_active(input_image, detection_area_size, x, y)) {
                    // Store its coordinates
                    add_to_cell_list(cell_list, x, y);

                    // Clear the area to prevent detecting the same cell again
                    clear_detection_area(input_image, detection_area_size, x, y);
                }
            }
        }
    }
}

char check_for_cell(unsigned char inputImage[BMP_WIDTH][BMP_HEIGHT], const int x, const int y) {
    for (int i = -6; i < 6; ++i) {
        if ((is_valid_coordinate(x + i,y - 6) && inputImage[x + i][y - 6]) ||
            (is_valid_coordinate(x + i,y + 6) && inputImage[x + i][y + 6])) {
            return false;
        }
        if ((is_valid_coordinate(x - 6,y + i) && inputImage[x - 6][y + i]) ||
            (is_valid_coordinate(x + 6,y + i) && inputImage[x + 6][y + i])) {
            return false;
        }
    }
    for (int i = -7; i < 7; ++i) {
        if ((is_valid_coordinate(x + i,y - 7) && inputImage[x + i][y - 7]) ||
            (is_valid_coordinate(x + i,y + 7) && inputImage[x + i][y + 7])) {
            return false;
        }
        if ((is_valid_coordinate(x - 7,y + i) && inputImage[x - 7][y + i]) ||
            (is_valid_coordinate(x + 7,y + i) && inputImage[x + 7][y + i])) {
            return false;
        }
    }
    return true;
}

int detect_cells_quick(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], Cell_list *cell_list) {
    int cellsDetected = 0;
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGHT; y++) {
            if (input_image[x][y]) {
                if (check_for_cell(input_image, x ,y) == true) {
                    cellsDetected++;
                    add_to_cell_list(cell_list, x, y);
                    for (int i = -8; i < 8; i++) {
                        for (int j = -8; j < 8; j++) {
                            if (!is_valid_coordinate(x+i, y +j)) continue;
                            input_image[x + i][y + j] = 0;
                        }
                    }
                }
            }
        }
    }
    return cellsDetected;
}

void draw_points(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], Cell_list *cell_list) {
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
