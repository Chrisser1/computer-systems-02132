#ifndef CELL_DETECTION_IMAGE_PROCESSING_H
#define CELL_DETECTION_IMAGE_PROCESSING_H

#include <stdbool.h>

#include "cbmp.h"

/**
 * @brief Converts an RGB image to a grayscale image.
 *
 * @param input_image The source RGB image.
 * @param output_image The destination grayscale image.
 */
void convert_to_grayscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS],
                          unsigned char output_image[BMP_WIDTH][BMP_HEIGHT]);

/**
 * @brief Converts a single-channel grayscale image to a three-channel RGB image.
 *
 * @param input_image The source grayscale image.
 * @param output_image The destination RGB image.
 */
void convert_to_RGB(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT],
                    unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);

/**
 * @brief Applies a convolution with a given square kernel to an image.
 *
 * @param image The input/output image buffer.
 * @param kernel A pointer to the kernel, stored as a 1D array.
 * @param kernel_size The dimension of the kernel (e.g., 3 for 3x3, 5 for 5x5).
 */
void apply_convolution(unsigned char image[BMP_WIDTH][BMP_HEIGHT], const int* kernel, int kernel_size);

/**
 * @brief Applies a standard 3x3 Gaussian kernel to blur the image.
 *
 * @param input_image The image to be blurred.
 */
void gaussian_blur_3x3(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT]);

/**
 * @brief Applies a standard 5x5 Gaussian kernel to blur the image.
 *
 * @param input_image The image to be blurred.
 */
void gaussian_blur_5x5(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT]);

/**
 * @brief Applies a sharpening kernel to the image to enhance edges.
 *
 * @param image The image to be sharpened.
 */
void sharpen_image(unsigned char image[BMP_WIDTH][BMP_HEIGHT]);

/**
 * @brief Calculates an optimal threshold value for a binary image using Otsu's method.
 *
 * @param input_image The grayscale image used to calculate the threshold.
 * @return The calculated optimal threshold value.
 */
unsigned char otsu_threshold_value(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT]);

/**
 * @brief Converts a grayscale image to a binary image based on a threshold.
 *
 * @param input_image The image to be binarized. Pixels above the threshold become 255 (white), others become 0 (black).
 * @param threshold The threshold value.
 */
void binary_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], int threshold);

/**
 * @brief Determines if a single white pixel should be eroded.
 *
 * According to the rules, a white pixel is eroded (turns black) if any of its
 * direct neighbors are black.
 *
 * @param input_image The binary image buffer.
 * @param x The x-coordinate of the pixel to check.
 * @param y The y-coordinate of the pixel to check.
 * @return True if the pixel should be eroded, false otherwise.
 */
static bool should_pixel_erode(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], int x, int y);

/**
 * @brief Applies one erosion pass to the binary image.
 *
 * @param input_image The binary image to be eroded.
 * @return True if any pixel was changed during erosion, false otherwise.
 */
bool erode_image(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT]);

// Linked list structure to store detected cell coordinates
typedef struct Cell {
    int x;
    int y;
    struct Cell* next;
} Cell;

typedef struct {
    int cell_amount;
    Cell* head;
} Cell_list;

/**
 * @brief Creates and initializes an empty cell list.
 * @return A pointer to the newly created Cell_list.
 */
Cell_list* create_cell_list();

/**
 * @brief Adds a new cell with given coordinates to the beginning of the list.
 *
 * @param list A pointer to the Cell_list to modify.
 * @param x The x-coordinate for the new cell.
 * @param y The y-coordinate for the new cell.
 */
void add_to_cell_list(Cell_list* list, int x, int y);

/**
 * @brief Frees all memory used by a cell list and its nodes.
 * @param cell_list A pointer to the Cell_list to destroy.
 */
void destroy_cell_list(Cell_list* cell_list);

/**
 * @brief Checks if the exclusion frame is clear of white pixels.
 *
 * @param input_image The binary image.
 * @param detection_area_size The size of the inner detection area.
 * @param exclusion_frame_thickness The thickness of the exclusion frame.
 * @param center_x The center x-coordinate of the detection window.
 * @param center_y The center y-coordinate of the detection window.
 *
 * @return True if the frame is all black, false otherwise.
 */
static bool is_exclusion_frame_clear(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], int detection_area_size,
                                     int exclusion_frame_thickness, int center_x, int center_y);

/**
 * @brief Checks if the inner detection area contains at least one white pixel.
 *
 * @param image The binary image.
 * @param detection_area_size The size of the detection area.
 * @param center_x The center x-coordinate of the detection window.
 * @param center_y The center y-coordinate of the detection window.
 * @return True if at least one white pixel is found, false otherwise.
 */
static bool is_detection_area_active(unsigned char image[BMP_WIDTH][BMP_HEIGHT], int detection_area_size,
                                     int center_x, int center_y);

/**
 * @brief Sets all pixels in a specified detection area to black (0).
 *
 * @param image The binary image to modify.
 * @param detection_area_size The size of the area to clear.
 * @param center_x The center x-coordinate of the area to clear.
 * @param center_y The center y-coordinate of the area to clear.
 */
static void clear_detection_area(unsigned char image[BMP_WIDTH][BMP_HEIGHT], int detection_area_size,
                                 int center_x, int center_y);

/**
 * @brief Detects cells by sliding a detection window across the image.
 * A cell is found if the inner detection area is active (has white pixels) and the surrounding
 * exclusion frame is clear (all black). Found cells are added to the list and their area is cleared.
 *
 * @param input_image The binary image to process.
 * @param detection_area_size The size of the inner detection window.
 * @param exclusion_frame_thickness The thickness of the surrounding exclusion frame.
 * @param cell_list The list to store coordinates of detected cells.
 */
void detect_cells(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT],
                  int detection_area_size, int exclusion_frame_thickness, Cell_list *cell_list);

/**
 * @brief Checks if the frames surrounding a pixel at (x, y) are clear of other white pixels.
 *
 * @param inputImage The binary image to check.
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @return True if the surrounding frame is clear, false otherwise.
 */
char check_for_cell(unsigned char inputImage[BMP_WIDTH][BMP_HEIGHT], int x, int y);

/**
 * @brief Performs a fast scan for cells by checking for isolated white pixels.
 * If an isolated pixel is found, it is added to the list and its surrounding area is cleared.
 *
 * @param input_image The binary image to process.
 * @param cell_list The list to store coordinates of detected cells.
 * @return The total number of cells detected.
 */
int detect_cells_quick(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], Cell_list *cell_list);

/**
 * @brief Draws a red cross marker on the RGB image for each cell in the list.
 *
 * @param input_image The output RGB image to draw on.
 * @param cell_list The list containing the coordinates of detected cells.
 */
void draw_points(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], Cell_list *cell_list);

#endif // CELL_DETECTION_IMAGE_PROCESSING_H