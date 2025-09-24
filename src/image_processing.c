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
static bool _is_valid_coordinate(const int x, const int y) {
    return x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGHT;
}