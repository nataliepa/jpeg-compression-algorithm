#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "func.h"
#include "write_file.h"
#include "buff.h"

#define FILEPATH_SIZE 1000
#define MAX_BUFFER_SIZE 65536

int main(int argc, char *argv[])
{
    int width, height, color_depth, no_8x8_blocks;
    char ppm_version[2];
    char input_file[FILEPATH_SIZE];
    char output_file[FILEPATH_SIZE];
    char *token;
    int **array_Y;
    int **zig_zag_Y, **rle_Y;
    int i, j;

    FILE *input, *output,*output_ppm;

    uint8_t **ppm_image;
    buffer_t *b = init_buffer(MAX_BUFFER_SIZE);

    if(argc < 2)
    {
        printf("Wrong number of arguments...");
        exit(-1);
    }

    //arguments
    strcpy(input_file, argv[1]);
    
    //create output filename
    strcpy(output_file, argv[1]);
    token = strtok(output_file, ".");
    strcat(output_file, ".jpeg");
    
    //open files
    input = open_file(input_file, 0);
    output_ppm = open_file("dct_lena.pgm", 1);
    output = open_file(output_file, 1);

    //read image
    read_image_header(input, &width, &height, &color_depth, ppm_version);

    //allocate memory for image array
    generic_2d_malloc((void***)&ppm_image, width, height, sizeof(uint8_t));
     //allocate memory for array_Y
    generic_2d_malloc((void***)&array_Y, width, height, sizeof(int));
    //allocate memory for zig_zag_Y
    no_8x8_blocks = width/8 * height/8;
    generic_2d_malloc((void***)&zig_zag_Y, 64, no_8x8_blocks, sizeof(int));
     //allocate memory for rle Y
    generic_2d_malloc((void***)&rle_Y, 72, no_8x8_blocks, sizeof(int));

    //no_8x8_blocks = width/8 * height/8;
    read_image_data(input, ppm_image, width, height);

    //copy ppm_image array in array_Y
    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            array_Y[i][j] = (int)ppm_image[i][j];
        }
    }

    //step 1: Discrete Cosine Tranform (DCT)
    perform_DCT(array_Y, width, height);
    
    //step 2: Quantization
    perform_Quantization(array_Y, width, height);

    //step 3: Zig-Zag scan
    perfom_zig_zag_scan(array_Y, zig_zag_Y, width, height);

    //step 4: DC dpcm
    dc_dpcm(zig_zag_Y, no_8x8_blocks);

    //step 5: AC Runlength Encoding (RLE)
    ac_rle(zig_zag_Y, rle_Y, no_8x8_blocks);

    //write bitstream in buffer b
    write_bitstream(b, rle_Y, no_8x8_blocks, width, height);

    //write bitstream in output file
    fwrite(b->stream, 1, b->length, output);

    //close input and output file
    fclose(input);
    fclose(output);

    return 0;
}