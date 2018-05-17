#ifndef IMAGE_H
#define IMAGE_H

//rgb components structure
typedef struct rgb
{
    //unsigned short int r, g, b;
    unsigned char r, g, b;
    //double y, u, v;
}rgb_t;

//rbg image structre
typedef struct image
{
    int width;
    int height;
    rgb_t **rgb_image_array;
}image_t;

//function to open a file in read or write mode
FILE* open_file(char filepath[], int mode);

//allocate memory for any type of 2d array
void generic_2d_malloc(void ***array, unsigned int width, unsigned int height, size_t size);

//function to read input image file
void read_image(FILE *fp, image_t *PPMimage, int *width, int *height, int *color_depth, char ppm_version[]);

//function to print image array
void print_image_array(image_t *PPMimage, int width, int height);

//write image info to output file
void write_image(FILE * fp, image_t *PPMimage, int width, int height, int color_depth, char ppm_version[]);

//convert from RGB to YUV
void yuv_to_rgb_conversion(image_t *PPMimage, int width, int height, float **array_Y, float **array_U, float **array_V);

//write yuv image in file
void write_yuv_image(FILE * fp, float **array_Y, float **array_U, float **array_V, int width, int height, int color_depth, char ppm_version[]);

//2x2 chroma subsampling
void chroma_subsampling(image_t *PPMimage, float **array_U, float **array_V, int width, int height, int color_depth, FILE *fp);

//perform DCT transform in Y, U and V channels
void perform_DCT(float **array_Y, float **array_U, float **array_V, int width, int height, FILE *fp);

//DCT tranform function
void DCT_transform(float array[8][8]);

//perform Quantization in each DCT transformed 8x8 block
void perform_Quantization(float **array_Y, float **array_U, float **array_V, int width, int height, FILE *fp, int Luminance_Q_array[][8], int Chrominance_Q_array[][8]);

//perform zig zag scan
void zig_zag_scan(float **array_Y, float **array_U, float **array_V, int **zig_zag_Y, int **zig_zag_U, int **zig_zag_V, int width, int height);

//zig zag scan algorithm for each 8x8 block
void perform_zig_zag(int array_temp[][8], int array_final[]);

#endif // !IMAGE_H