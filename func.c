#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "func.h"
#include "huffman_tables.h"


#define pi 3.14159265

FILE* open_file(char filepath[], int mode)
{
    FILE *fp;

    if(mode == 0) //open file in read mode
        fp = fopen(filepath, "rb");
    else if(mode == 1) //open file in write mode
        fp = fopen(filepath, "wb");
    else
        fp = fopen(filepath, "ab");

    if(fp == NULL)
    {
        printf("Could not open file...!");
        exit(1);
    }
    return fp;
}

void generic_2d_malloc(void ***array, unsigned int width, unsigned int height, size_t size)
{
    int i;

    *array = malloc(sizeof(void*) * height);
    for (i=0; i < height; i++) {
        (*array)[i] = malloc(size * width);
    }
}

void read_image_header(FILE *fp, int *width, int *height, int *color_depth, char ppm_version[])
{
    char c, buff[10], size;
    int i, j;
    

    //read header
    fscanf(fp, "%s", buff);
    strcpy(ppm_version, buff);
    if(!strcmp(ppm_version, "P5"))
        printf("PPM file (raw version - P5).\n");
    else if(!strcmp(buff, "P2"))
        printf("PPM file (plain text version - P2).\n");
    else
    {
        printf("Input file is not a PPM file...\n");
        exit(-1);
    }

    //discard any commment after header
    c = getc(fp);
    if(c == '\n')
    {
        c = getc(fp);
        while(c == '#')
            fgets(buff, sizeof(buff), fp);
    }
    ungetc(c,fp);

    //read width, height and color depth
    fscanf(fp, "%d %d %d", &(*width), &(*height), &(*color_depth));
    printf("width is: %d\nheight is: %d\ncolor depth is:%d\n", *width, *height, *color_depth);/*convert in power of 2*/
    fscanf(fp, "%c", &c);
    
    
}

void read_image_data(FILE *fp, uint8_t **PPMimage, int width, int height)
{
    int i, j;

     //read pixel values
    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            PPMimage[i][j] = getc(fp);   
        }
    }
}

void perform_DCT(int **array_Y, int width, int height)
{
    int i, j, k, l, m = 0, n = 0;
    int array_Y_8x8[8][8], array_dct[8][8];
    
    for(i=0; i<height; i+=8)
    {
        for(j=0; j<width; j+=8)
        {
            m=0;
            for(k=i; k<8+i; k++)
            {
                n = 0;
                for(l=j; l<8+j; l++)
                {
                    array_Y_8x8[m][n] = (int)array_Y[k][l] - 128;
                    n++;
                }
                m++;
            }
            dctTransform(array_Y_8x8, array_dct);
            m=0;
            for(k=i; k<8+i; k++)
            {
                n = 0;
                for(l=j; l<8+j; l++)
                {
                    array_Y[k][l] = array_dct[m][n];
                    n++;
                }
                m++;
            }
           
        }
       
    }
    

    
    
}

// Function to find discrete cosine transform and print it
int dctTransform(int matrix[][8], int dct_array[][8])
{
    int i, j, k, l;
 
 
    float ci, cj, dct1, sum;
 
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
 
            // ci and cj depends on frequency as well as
            // number of row and columns of specified matrix
            if (i == 0)
                ci = 1 / sqrt(2.0);
            else
                ci = 1;
            if (j == 0)
                cj = 1 / sqrt(2.0);
            else
                cj = 1;
 
            // sum will temporarily store the sum of 
            // cosine signals
            sum = 0.0;
            for (k = 0; k < 8; k++) {
                for (l = 0; l < 8; l++) {
                    dct1 = matrix[k][l] * 
                           cos((2 * k + 1) * i * pi / 16) * 
                           cos((2 * l + 1) * j * pi / 16);
                    sum = sum + dct1;
                }
            }
            dct_array[i][j] =round(0.25 * ci * cj * sum);
        }
    }
}

void print_image_array(uint8_t **PPMimage, int width, int height)
{
    int i,j;

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            printf("%d\t", PPMimage[i][j]);
        }
        printf("\n");
    }

}

void perform_Quantization(int **array_Y, int width, int height)
{
    int i, j, k, l, m, n;

    for(i=0; i<height; i+=8)
    {
        for(j=0; j<width; j+=8)
        {
            m=0;
            for(k=i; k<8+i; k++)
            {
                n = 0;
                for(l=j; l<8+j; l++)
                {
                    array_Y[k][l] = round(((float)array_Y[k][l] / Luminance_Q_array[m][n]));
                    n++;
                }
                m++;
            }
           
        }
    }


}


void print_Y_array(int **array, int width, int height)
{
    int i,j;

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            printf("%d\t", array[i][j]);
        }
        printf("\n");
    }


}

void write_ppm_file(int **array, int width, int height, int color_depth, char ppm_version[], FILE *fp)
{
    int i, j;

    fprintf(fp, "%s\n%d %d\n%d\n", ppm_version, width, height, color_depth);

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            fprintf(fp, "%c", (uint8_t)array[i][j] + 127);
        }
    }
}

void perfom_zig_zag_scan(int **array, int **zig_zag, int width,int height)
{
    int i, j, m, n, k, l, x = 0, y, a = 0, b = 0;
    int temp_array[8][8], final_array[64];

    for(i=0; i<height; i+=8)
    {
        for(j=0; j<width; j+=8)
        {
            m=0;
            for(k=i; k<8+i; k++)
            {
                n = 0;
                for(l=j; l<8+j; l++)
                {
                    temp_array[m][n] = (int)array[k][l];
                    n++;
                    
                }
                m++;
            }
            zig_zag_scan(temp_array, final_array);
            for(y=0; y<64; y++)
                zig_zag[x][y] =  final_array[y];
            x++;
            
            
        }
       
    }
    
}

void zig_zag_scan(int array_temp[][8], int array_final[])
{
    int i = 0, j = 0, k = 0, count = 0;

    array_final[k] = array_temp[i][j];
    count++;
    while(i!=8-1)
    {
        if(i == 0)
        {
            array_final[++k] = array_temp[i][++j];
            count++;
            while(j!=0)
            {
                array_final[++k] = array_temp[++i][--j];
                count++;
            }
        }
        if(j == 0 && i!=8-1)
        {
            array_final[++k] = array_temp[++i][j];
            count++;
            while(i!=0)
            {
                array_final[++k] = array_temp[--i][++j];
                count++;
            }
        }
    }
    while(i!=8-1 || j!=8-1)
    {
        if(i == 8-1)
        {
            array_final[++k] = array_temp[i][++j];
            count++;
            while(j!=8-1)
            {
                array_final[++k] = array_temp[--i][++j];
                count++;
            }
        }
        if(j == 8-1 && i!= 8-1)
        {
            array_final[++k] = array_temp[++i][j];
            count++;
            while(i!=8-1)
            {
                array_final[++k] = array_temp[++i][--j];
                count++;
            }
        }


    }

}

void dc_dpcm(int **zig_zag, int no_8x8_blocks)
{
    int i, temp1, temp2;


    temp2 = zig_zag[0][0];
    for(i=1; i<no_8x8_blocks; i++)
    {
        temp1 = zig_zag[i][0];
        zig_zag[i][0] = zig_zag[i][0] - temp2;
        temp2 = temp1;
    }
}

void ac_rle(int **zig_zag_array, int **rle_array, int no_8x8_blocks)
{
    int i, j, k = 0, count = 0; 

    for(i=0; i<no_8x8_blocks; i++)
    {
        rle_array[i][0] = zig_zag_array[i][0];
        for(j=1; j<72; j++)
        {
            rle_array[i][j] = -1;
        }
    }


    //rle in Y component
    for(i=0; i<no_8x8_blocks; i++)
    {
        k = 0;
        for(j=1; j<64; j++)
        {
            if(zig_zag_array[i][j] == 0)
            {
                count = 0;
                while(zig_zag_array[i][j] == 0 && j<64)
                {
                    count++;
                    j++;
                }
                if(j==64)
                {
                    rle_array[i][++k] = 0;
                    rle_array[i][++k] = 0;
                }
                else
                {
                    rle_array[i][++k] = 0;
                    rle_array[i][++k] = count;
                }
                j--;

            }
            else
            {
                rle_array[i][++k] = zig_zag_array[i][j];
                if(j==63)
                {
                    rle_array[i][++k] = INT_MAX;
                    j=64;
                }
            }
           
        }
    }

}

void init_array(int array[], int size)
{
    int i;
    for(i=0; i<size; i++)
    {
        array[i] = -1;
    }
}

void convert_dec_to_binary(int number, int array[], int bits_num)
{
    int i = 0, n = abs(number);

    if(number==0)
        return;

    while(n != 0)
    {
        array[bits_num - i - 1] = n % 2;
        n = n / 2;
        i++;
    }

    if(number < 0)
    {
        for(i=0; i<bits_num; i++)
        {
            if(array[i]==0)
                array[i] = 1;
            else
                array[i] = 0;
        }
    }

}

void convert_string_to_bit(char array[], int size, int bit_array[])
{
    int i;

    for(i=0; i<size; i++)
    {
        bit_array[i] = (int)array[i] - 48;
    }
}

void encode_DC_value(int value, int value_DC[], int size_DC[], int *value_bits, int *code_bits)
{
    if(value == 0)
        *value_bits = 0;
    else
        *value_bits = floor( log2(abs(value)) + 1 ); 
    
    convert_dec_to_binary(value, value_DC, *value_bits);

    convert_string_to_bit(DC_Luminance[*value_bits].code_word, DC_Luminance[*value_bits].code_length, size_DC);

    *code_bits = DC_Luminance[*value_bits].code_length;
}

void encode_AC_value(int value, int z_runlenght, int value_AC[], int size_AC[], int *value_bits, int *code_bits)
{
    if(value == 0)
        *value_bits = 0;
    else
        *value_bits = floor( log2(abs(value)) + 1 );

    if(z_runlenght == 16)
    {
        convert_string_to_bit(AC_Luminance[15][0], Lum_code_len[15][0], size_AC);
        *code_bits = Lum_code_len[15][0];
    }
    else
    {
        convert_dec_to_binary(value, value_AC, *value_bits);

        if(z_runlenght > 0)
        {
            convert_string_to_bit(AC_Luminance[z_runlenght][*value_bits], Lum_code_len[z_runlenght][*value_bits], size_AC);
            *code_bits = Lum_code_len[z_runlenght][*value_bits];
        }
        else if(z_runlenght == 0)
        {
            convert_string_to_bit(AC_Luminance[0][*value_bits], Lum_code_len[0][*value_bits], size_AC);
            *code_bits = Lum_code_len[0][*value_bits];
        }
    }
}

void flush_bits(int size_array[], int bits_array[], int final_array[], int buffer[], int bits, int code_length)
{
    int i, j, pos_b = 0, pos_f = 0, pos_s = 0, pos_bit = 0, temp;

    //first fill final array with any remaining bits of buffer
    j = 0;
    for(i=0; i<8; i++)
    {
        if(final_array[i]==-1 && buffer[j]!=-1)
        {
            final_array[i] = buffer[j];
            buffer[j] = -1;
            j++;
        }
    }
   
    //normalize buffer
    normalize_buffer(buffer, 128, j);
    for(i=0; i<128; i++)
    {
        if(buffer[i]==-1)
        {
            pos_b = i;
            break;
        }
        if(i==127)
            pos_b = 128;
    }
    //check if final array is full and keep track of first empty position
    for(i=0; i<8; i++)
    {
        if(final_array[i] == -1)
        {
            pos_f = i;
            break;
        }
        if(i==7)
            pos_f = 8;
           
    }
    //start filling final array with size array
    temp = pos_f;
    j=0;
    for(i=pos_f; i<8; i++)
    {
        if(i < (code_length + temp))
        {
            final_array[i] = size_array[j];
            j++;
            pos_f = i;
        }
        else
        {

            break;
        }
    }
    //fill buffer with remaining bits of size_array, if any..
    for(i=j; i<code_length; i++)
    {
        buffer[pos_b] = size_array[i];
        pos_b++;
    }    

    //fill final array with bits_array
    j=0;
    for(i=pos_f+1; i<8; i++)
    {
        if(i<(bits + pos_f+1))
        {
            final_array[i] = bits_array[j];
            j++;
        }
        else
        {
            break;
        }
    }

    //fill buffer with remaining bits of bits_array, if any..
    for(i=j; i<bits; i++)
    {
        buffer[pos_b] = bits_array[i];
        pos_b++;
    }
    

}

void normalize_buffer(int array[], int size, int pos)
{
    int i, j = 0, temp[128];

    init_array(temp, size);


    for(i=pos; i<size; i++)
    {
        if(array[i]!=-1)
        {
            temp[j] = array[i];
            j++;
        }
    }

    for(i=0; i<size; i++)
    {
        array[i] = temp[i];
    }
}

int is_byte(int array[], int size)
{
    //check if final array is full
    int i;
    for(i=0; i<size; i++)
    {
        if(array[i] == -1)
            return 0;
    }
    return 1;
}

int convert_binary_to_dec(int array[], int size)
{
    int i, number = 0, j = 0;

    for(i=size-1; i>=0; i--)
    {
        number += array[i] * pow(2, j);
        j++;
    }
    return number;
}

void write_compressed_data(uint8_t value[], FILE *fp)
{
    uint8_t buf[1];
    buf[0] = 0;
    if(value[0] == 255)
    {
        fwrite(value, 1, 1, fp);
        fwrite(buf, 1, 1, fp);
        return;
    }
    fwrite(value, 1, 1, fp);
}
