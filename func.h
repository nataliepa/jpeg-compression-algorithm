#ifndef FUNC_H
#define FUNC_H


//function to open a file in read or write mode
FILE* open_file(char filepath[], int mode);

//function to read PPM image header data
void read_image_header(FILE *fp, int *width, int *height, int *color_depth, char ppm_version[]);

//function to read pixel values
void read_image_data(FILE *fp, uint8_t **PPMimage, int width, int height);

//function to allocate memory for a 2d array
void generic_2d_malloc(void ***array, unsigned int width, unsigned int height, size_t size);

//function to print image array
void print_image_array(uint8_t **PPMimage, int width, int height);

void print_Y_array(int **array, int width, int height);

void write_ppm_file(int **array, int width, int height, int color_depth, char ppm_version[], FILE *fp);

//perform dct tranform in each 8x8 block
void perform_DCT(int **array_Y, int width, int height);

//dct transform
int dctTransform(int matrix[][8], int dct_array[][8]);

//function to perform Quatization in each 8x8 block
void perform_Quantization(int **array_Y, int width, int height);

//function to perform zig-zag scan in each 8x8 block
void perfom_zig_zag_scan(int **array, int **zig_zag, int width,int height);

//zig-zag scan
void zig_zag_scan(int array_temp[][8], int array_final[]);

//funtion to perform DC dpcm in each 8x8 block
void dc_dpcm(int **zig_zag, int no_8x8_blocks);

//funtion to perform AC Runlength Encoding in each 8x8 block
void ac_rle(int **zig_zag_array, int **rle_array, int no_8x8_blocks);

//funtion to encode each DC value
void encode_DC_value(int value, int value_DC[], int size_DC[], int *value_bits, int *code_bits);

//funtion to encode each AC value
void encode_AC_value(int value, int z_runlenght, int value_AC[], int size_AC[], int *value_bits, int *code_bits);

//function to flush bits into final and buffer array 
void flush_bits(int size_array[], int bits_array[], int final_array[], int buffer[], int bits, int code_length);

//write compressed data to output file
void write_compressed_data(uint8_t value[], FILE *fp);

//function to check if final array is a byte
int is_byte(int array[], int size);

//funtion to convert a decimal number to binary
void convert_dec_to_binary(int number, int array[], int bits_num);

//funtion to conver a string array to binary(integer) array
void convert_string_to_bit(char array[], int size, int bit_array[]);

//funtion to initialize an array with -1 value
void init_array(int array[], int size);

//function normalize buffer array
void normalize_buffer(int array[], int size, int pos);

//function to convert a binary number to a decimal number
int convert_binary_to_dec(int array[], int size);


#endif
