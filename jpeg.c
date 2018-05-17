#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "image.h"

#define FILEPATH_SIZE 1000



int main(int argc, char *argv[])
{
    FILE *fp, *fp1;

    int i, j;

    int Luminance_Q_array[8][8] = {{16, 11, 10, 16, 24, 40, 51, 61},
                                   {12, 12, 14, 19, 26, 58, 60, 55},
                                   {14, 13, 16, 24, 40, 57, 69, 56},
                                   {14, 17, 22, 29, 51, 87, 80, 62},
                                   {18, 22, 37, 56, 68, 109, 103, 77},
                                   {24, 35, 55, 64, 81, 104, 113, 92},
                                   {49, 64, 78, 87, 103, 121, 120, 101},
                                   {72, 92, 95, 98, 112, 100, 103, 99}};

    int Chrominance_Q_array[8][8] = {{17, 18, 24, 47, 99, 99, 99, 99},
                                     {18, 21, 26, 66, 99, 99, 99, 99},
                                     {24, 26, 56, 99, 99, 99, 99, 99},
                                     {47, 66, 99, 99, 99, 99, 99, 99},
                                     {99, 99, 99, 99, 99, 99, 99, 99},
                                     {99, 99, 99, 99, 99, 99, 99, 99},
                                     {99, 99, 99, 99, 99, 99, 99, 99},
                                     {99, 99, 99, 99, 99, 99, 99, 99}};

    image_t *PPMimage;

    float **array_Y, **array_U, **array_V;
    int **zig_zag_Y, **zig_zag_U, **zig_zag_V;
    
    char input_file[FILEPATH_SIZE];
    char output_file[FILEPATH_SIZE];

    PPMimage = malloc(sizeof(image_t)); 

    int width, height, color_depth, no_8x8_blocks;
    char ppm_version[2];

    //check validity of number of arguments
    if(argc < 3)
    {
        printf("Wrong number of arguments...");
        exit(-1);
    }
    //arguments
    strcpy(input_file, argv[1]);
    strcpy(output_file, argv[2]);
    
    //open files
    fp = open_file(input_file, 0);
    fp1 = open_file(output_file, 1);

    //read ppm image
    read_image(fp, PPMimage, &width, &height, &color_depth, ppm_version);
    
    //allocate memory for temp Y, U, V arrays
    generic_2d_malloc((void***)&array_Y, width, height, sizeof(float));
    generic_2d_malloc((void***)&array_U, width, height, sizeof(float));
    generic_2d_malloc((void***)&array_V, width, height, sizeof(float));

    //allocate memory for zig zag Y, U, and V;
    no_8x8_blocks = width/8 * height/8;
    generic_2d_malloc((void***)&zig_zag_Y, 64, no_8x8_blocks, sizeof(int));
    generic_2d_malloc((void***)&zig_zag_U, 64, no_8x8_blocks/4, sizeof(int));
    generic_2d_malloc((void***)&zig_zag_V, 64, no_8x8_blocks/4, sizeof(int));
    
    //convert RGB to YUV
    yuv_to_rgb_conversion(PPMimage, width, height, array_Y, array_U, array_V);
   
    //perform subsampling to U and V channels
    chroma_subsampling(PPMimage, array_U, array_V, width, height, color_depth, fp1);
    
    //perform DCT in each 8x8 block
    perform_DCT(array_Y, array_U, array_V, width, height, fp1);

    //perform Quantization in each DCT transformed 8x8 block
    perform_Quantization(array_Y, array_U, array_V, width, height, fp1, Luminance_Q_array, Chrominance_Q_array);

    //perform zig zag scan in Y, U and V
    zig_zag_scan(array_Y, array_U, array_V, zig_zag_Y, zig_zag_U, zig_zag_V, width, height);

    
    fclose(fp);
    fclose(fp1);
    return 0;
}
