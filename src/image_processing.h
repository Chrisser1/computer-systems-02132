#ifndef CEll_DETECTION_IMAGE_PROCESSING_H
#define CEll_DETECTION_IMAGE_PROCESSING_H

#include <stdbool.h>

#include "cbmp.h"

/**
 * @brief Converts RGB image to gray scale
 *
 *
 * @param input_image The input image.
 * @param output_image The output image.
 */
void convert_to_grayscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS],
    unsigned char output_image[BMP_WIDTH][BMP_HEIGHT]);

/**
 *
 *
 * @param input_image
 * @param output_image
 */
void convert_to_RGB(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT],
    unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);

/**
 *
 * @param input_image
 * @param threshold
 */
void binary_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], int threshold);

/**
 * @brief Determines if a single white pixel should be eroded.
 *
 * According to the rules, a white pixel is eroded (turns black) if any of its
 * neighbors defined by the structuring element are black. A neighbor outside
 * the image boundary is treated as black.
 *
 * @param input_image The binary image buffer.
 * @param x The x-coordinate of the pixel to check.
 * @param y The y-coordinate of the pixel to check.
 * @return True if the pixel should be eroded, false otherwise.
 */
static bool should_pixel_erode(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], int x, int y);

/**
 *
 * @param input_image
 * @return
 */
bool erode_image(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT]);

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
 * @brief Adds a new cell to the beginning of the list.
 *
 * @param list A pointer to the Cell_list to modify.
 * @param x The x-coordinate for the new cell.
 * @param y The y-coordinate for the new cell.
 */
void add_to_cell_list(Cell_list* list, int x, int y);

/**
 * @brief Frees all memory used by the list and its cells.
 * @param cell_list A pointer to the Cell_list to destroy.
 */
void destroy_cell_list(Cell_list* cell_list);

/**
 * @brief Checks if the exclusion frame is clear of white pixels.
 *
 * @param input_image The binary image.
 * @param detection_area_size
 * @param exclusion_frame_thickness The exclusion frame size.
 * @param center_x The center x-coordinate of the detection window.
 * @param center_y The center y-coordinate of the detection window.
 *
 * @return True if the frame is all black, false otherwise.
 */
static bool is_exclusion_frame_clear(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], int detection_area_size,
                                     int exclusion_frame_thickness, int center_x, int center_y);

/**
 *
 * @param input_image
 * @param detection_area_size
 * @param exclusion_frame_thickness
 * @param cell_list
 * @return
 */
unsigned int detect_cells(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT],
                          int detection_area_size, int exclusion_frame_thickness, Cell_list *cell_list);

/**
 *
 * @param input_image
 * @param cell_list
 */
void draw_points(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], Cell_list *cell_list);

#endif // CEll_DETECTION_IMAGE_PROCESSING_H



