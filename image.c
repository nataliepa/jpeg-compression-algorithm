#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "image.h"

#define PI 3.14159265

FILE* open_file(char filepath[], int mode)
{
    FILE *fp;

    if(mode == 0) //open file in read mode
        fp = fopen(filepath, "rb");
    else //open file in write mode
        fp = fopen(filepath, "wb");

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

void read_image(FILE *fp, image_t *PPMimage, int *width, int *height, int *color_depth, char ppm_version[])
{
    char c, buff[10], size;
    int i, j;
    

    //read header
    fscanf(fp, "%s", buff);
    strcpy(ppm_version, buff);
    if(!strcmp(ppm_version, "P6"))
        printf("PPM file (raw version - P6).\n");
    else if(!strcmp(buff, "P3"))
        printf("PPM file (plain text version - P3).\n");
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
    
    //allocate memory for image array
    generic_2d_malloc((void***)&PPMimage->rgb_image_array, *width, *height, sizeof(rgb_t));
    //&(*(PPMimage->rgb_image_array))
    //read pixel values
    for(i=0; i<*height; i++)
    {
        for(j=0; j<*width; j++)
        {
            //fscanf(fp, "%hu%hu%hu", &PPMimage->image_array[i][j].r, &PPMimage->image_array[i][j].g, &PPMimage->image_array[i][j].b);
            PPMimage->rgb_image_array[i][j].r = getc(fp);
            PPMimage->rgb_image_array[i][j].g = getc(fp);
            PPMimage->rgb_image_array[i][j].b = getc(fp);
        }
    }
    
    
}

void print_image_array(image_t *PPMimage, int width, int height)
{
    int i,j;

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            printf("%d %d %d\t", PPMimage->rgb_image_array[i][j].r, PPMimage->rgb_image_array[i][j].g, PPMimage->rgb_image_array[i][j].b);
        }
        printf("\n");
    }

}

void write_image(FILE *fp, image_t *PPMimage, int width, int height, int color_depth, char ppm_version[])
{
    int i, j;

    fprintf(fp, "%s\n", ppm_version);
    fprintf(fp, "%d %d\n%d\n", width, height, color_depth);

   for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            //fprintf(fp, "%u%u%u", PPMimage->image_array[i][j].r, PPMimage->image_array[i][j].g, PPMimage->image_array[i][j].b);
            putc(PPMimage->rgb_image_array[i][j].r, fp);
            putc(PPMimage->rgb_image_array[i][j].g, fp);
            putc(PPMimage->rgb_image_array[i][j].b, fp);
        }
    }   
        

}

void yuv_to_rgb_conversion(image_t *PPMimage, int width, int height, float **array_Y, float **array_U, float **array_V)
{
    int i, j;

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            array_Y[i][j] = PPMimage->rgb_image_array[i][j].r * .299000 + PPMimage->rgb_image_array[i][j].g * .587000 + PPMimage->rgb_image_array[i][j].b  * .114000;
            array_U[i][j] = PPMimage->rgb_image_array[i][j].r * -.168736 + PPMimage->rgb_image_array[i][j].g * -.331264 +  PPMimage->rgb_image_array[i][j].b * .500000 + 128;
            array_V[i][j] = PPMimage->rgb_image_array[i][j].r * .500000 + PPMimage->rgb_image_array[i][j].g * -.418688 +  PPMimage->rgb_image_array[i][j].b * -.081312 + 128;
        }
    }   
}

void write_yuv_image(FILE * fp, float **array_Y, float **array_U, float **array_V, int width, int height, int color_depth, char ppm_version[])
{
    int i, j;

    fprintf(fp, "%s\n", ppm_version);
    fprintf(fp, "%d %d\n%d\n", width, height, color_depth);

   for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            //fprintf(fp, "%u%u%u", PPMimage->image_array[i][j].r, PPMimage->image_array[i][j].g, PPMimage->image_array[i][j].b);
            putc((unsigned char)array_Y[i][j], fp);
            putc((unsigned char)array_U[i][j], fp);
            putc((unsigned char)array_V[i][j], fp);
        }
    }   
        

}

void chroma_subsampling(image_t *PPMimage, float **array_U, float **array_V, int width, int height, int color_depth, FILE *fp)
{
    int i, j, k = 0, l = 0, m = 0, n = 0, c=0;
    float avg_U_sample, avg_V_sample, sum_U = 0, sum_V = 0;

    //array_U = allocate_yuv_image_memory(array_U, width/2, height/2, 1); //realloc array_U memory
    //array_V = allocate_yuv_image_memory(array_V, width/2, height/2, 1); //realloc array_V memory
    
    //perform chroma subsampling for U and V channels
    for(i=0; i<height; i+=2)
    {
        n=0;
        for(j=0; j<width; j+=2)
        {
            sum_U = 0; sum_V = 0;
            for(k=i; k<2+i; k++)
            {
                for(l=j; l<2+j; l++)
                {
                    sum_U = sum_U + array_U[k][l];
                    sum_V = sum_V + array_V[k][l];
                }
            }
            avg_U_sample = sum_U / 4;
            avg_V_sample = sum_V / 4;
            array_U[m][n] = avg_U_sample;
            array_V[m][n] = avg_V_sample;
            
            n++;
        }
        m++;
    }

   /* fprintf(fp, "%s\n", "P5");
    fprintf(fp, "%d %d\n%d\n", width/2, height/2, color_depth);

    //test output of subsampled image
    for(i=0; i<height/2; i++)
    {
        for(j=0; j<width/2; j++)
        {
            //fprintf(fp, "%u%u%u", PPMimage->image_array[i][j].r, PPMimage->image_array[i][j].g, PPMimage->image_array[i][j].b);
            putc((unsigned char)PPM_subsampled_image->yuv_image_array[i][j].v, fp);
            //putc((unsigned char)PPM_subsampled_image->yuv_image_array[i][j].u, fp);
        }
    }  
    */
   
}

void perform_DCT(float **array_Y, float **array_U, float **array_V, int width, int height, FILE *fp)
{
    int i, j, k, l, m = 0, n = 0;
    float array_Y_8x8[8][8], array_U_8x8[8][8], array_V_8x8[8][8];

    //fprintf(fp, "%s\n", "P5");
    //fprintf(fp, "%d %d\n%d\n", width, height, 255);
    
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
                    array_Y_8x8[m][n] = array_Y[k][l] - 128;
                    if((i<height/2) && (j<width/2))
                    {
                        array_U_8x8[m][n] = array_U[k][l] - 128;
                        array_V_8x8[m][n] = array_V[k][l] - 128;
                    }
                    n++;
                }
                m++;
            }
            DCT_transform(array_Y_8x8);
            if((i<height/2) && (j<width/2))
            {
                DCT_transform(array_U_8x8);
                DCT_transform(array_V_8x8);
            }
            m=0;
            for(k=i; k<8+i; k++)
            {
                n = 0;
                for(l=j; l<8+j; l++)
                {
                    array_Y[k][l] = array_Y_8x8[m][n];
                    if((i<height/2) && (j<width/2))
                    {
                        array_U[k][l] = array_U_8x8[m][n];
                        array_V[k][l] = array_V_8x8[m][n];
                    }
                    n++;
                }
                m++;
            }
           
        }
       
    }
    
    /*for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
           putc((unsigned char)PPM_subsampled_image->yuv_image_array[i][j].y , fp);
        }
   }  */
}

void DCT_transform(float array[][8])
{
    int i, j, k, l;
    double sum, dct, Au, Av;
    
    for(i=0; i<8; i++)
    {
        for(j=0; j<8; j++)
        {
            if(i == 0){
                Au = 1.0 / sqrt(2.0);
            }
            else{
                 Au = 1.0;
            }
               
            
            if(j == 0){
                Av = 1.0 / sqrt(2.0);
            }  
            else{
                Av = 1.0;
            }

            sum = 0.0;
            for(k=0; k<8; k++)
            {
                for(l=0; l<8; l++)
                {
                    //printf("lalla");
                    dct = array[k][l] * cos((2*k + 1) * i * PI / 16.0) * cos((2*l + 1) * j * PI / 16.0);
                    sum += dct;
                }
            }
            array[i][j] = 0.25 * Au * Av * sum;
        }
    }
}

void perform_Quantization(float **array_Y, float **array_U, float **array_V, int width, int height, FILE *fp, int Luminance_Q_array[][8], int Chrominance_Q_array[][8])
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
                    array_Y[k][l] = round(array_Y[k][l] / Luminance_Q_array[m][n]);
                    if((i<height/2) && (j<width/2))
                    {
                        array_U[k][l] = round(array_U[k][l] / Luminance_Q_array[m][n]);
                        array_V[k][l] = round(array_V[k][l] / Luminance_Q_array[m][n]);
                    }
                    n++;
                }
                m++;
            }
        }
       
    }

    fprintf(fp, "%s\n", "P5");
    fprintf(fp, "%d %d\n%d\n", width, height, 255);

    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
           putc((unsigned char)array_Y[i][j] , fp);
        }
   }  

   /*for(i=0; i<height/2; i++)
    {
        for(j=0; j<width/2; j++)
        {
           printf("%d ", (int)array_U[i][j]);
        }
        printf("\n");
   }*/  
}

void zig_zag_scan(float **array_Y, float **array_U, float **array_V, int **zig_zag_Y, int **zig_zag_U, int **zig_zag_V, int width,int height)
{
    int i, j, m, n, k, l, x = 0, y, a = 0, b = 0;
    int temp_array_Y[8][8], temp_array_U[8][8], temp_array_V[8][8],final_array[64];

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
                    temp_array_Y[m][n] = (int)array_Y[k][l];
                    if((i<height/2) && (j<width/2))
                    {
                        temp_array_U[m][n] = (int)array_U[k][l];
                        //printf("%d ", temp_array_U[m][n]);
                        temp_array_V[m][n] = (int)array_V[k][l];
                    }
                    n++;
                    
                }
                m++;
                //if((i<height/2) && (j<width/2))
                //{
                //    printf("\n");
                //}
            }
            perform_zig_zag(temp_array_Y, final_array);
            for(y=0; y<64; y++)
                zig_zag_Y[x][y] =  final_array[y];
            x++;
            if((i<height/2) && (j<width/2))
            {
                perform_zig_zag(temp_array_U, final_array);
                for(y=0; y<64; y++)
                    zig_zag_U[a][y] =  final_array[y];
                a++;
                perform_zig_zag(temp_array_V, final_array);
                for(y=0; y<64; y++)
                    zig_zag_V[b][y] =  final_array[y];
                b++;
            }
            
            
        }
       
    }
    for(i=0; i<(width/16*height/16); i++)
    {
        for(j=0; j<64; j++)
        {
            printf("%d ", zig_zag_U[i][j]);
        }
        printf("\n");
    }
        
    
}

void perform_zig_zag(int array_temp[][8], int array_final[])
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

    
    //printf("i is:%d\nj is:%d\n", i, j);
    //printf("count: %d\n", count);
}