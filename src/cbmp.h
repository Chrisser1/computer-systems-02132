#ifndef OS_CHALLENGE_CBMP_H
#define OS_CHALLENGE_CBMP_H

// Image dimensions
#define BMP_WIDTH 950
#define BMP_HEIGHT 950
#define BMP_CHANNELS 3

// Function to read a bitmap file
void read_bitmap(char* input_file_path, unsigned char output_image_array[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);

// Function to write a bitmap file
void write_bitmap(unsigned char input_image_array[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], char* output_file_path);


#endif //OS_CHALLENGE_CBMP_H