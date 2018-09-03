#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>

#include "write_file.h"
#include "standard_tables.h"
#include "func.h"

#define SOI_MARKER 0xFFD8 //Start Of Image marker 
#define EOI_MARKER 0xFFD9 //End of image marker
#define APP0_MARKER 0xFFE0 //APP0 marker
#define DQT_MARKER 0xFFDB //Define Quantization Table marker
#define SOF_MARKER 0xFFC0 //Start Of Frame marker
#define SOS_MARKER 0xFFDA //Start Of Scan marker
#define DHT_MARKER 0xFFC4 //Define Huffman Table marker
#define ZERO 0x00
#define DC_LENGTH 31
#define AC_LENGTH 181
#define DC_LUM_DEST 0x00
#define DC_CHR_DEST 0x01
#define AC_LUM_DEST 0x10
#define AC_CHR_DEST 0x11


void write_soi(buffer_t *b)
{
    write_2bytes_buffer(b, SOI_MARKER); //Start Of Image marker
}

void write_app0(buffer_t *b)
{
    write_2bytes_buffer(b, APP0_MARKER); //App0 marker
    write_2bytes_buffer(b, 0x0010); //length
    write_byte_buffer(b, 0x4A); //'J'
    write_byte_buffer(b, 0x46); //'F'
    write_byte_buffer(b, 0x49); //'I'
    write_byte_buffer(b, 0x46); //'F'
    write_byte_buffer(b, 0x00); //'\0'
    write_2bytes_buffer(b, 0x0101); //version 1.1
    write_byte_buffer(b, 0x00);
    write_2bytes_buffer(b, 0x0001); //X density = 1 
    write_2bytes_buffer(b, 0x0001); // Y density = 1 
    write_byte_buffer(b, 0x00);     // thumbnail width = 0 
    write_byte_buffer(b, 0x00); // thumbnail height = 0 

}

void write_dqt(buffer_t *b, uint8_t array[])
{
    int i;

    write_2bytes_buffer(b, DQT_MARKER); //DQT marker
    write_2bytes_buffer(b, 0x0043); // segment length
    write_byte_buffer(b, 0x00); // table 0, 8-bit precision (0)
    for(i=0; i<64; i++)
        write_byte_buffer(b, array[zz[i]]); 
}

void write_sof(buffer_t *b, uint16_t width, uint16_t height)
{
    write_2bytes_buffer(b, SOF_MARKER); //SOF marker
    write_2bytes_buffer(b, 0x000B); //length
    write_byte_buffer(b, 0x08); //8-bit precision
    write_2bytes_buffer(b, height);
    write_2bytes_buffer(b, width);
    write_byte_buffer(b, 0x01); // grayscale image
    write_byte_buffer(b, 0x01); // component ID = 1
    write_byte_buffer(b, 0x11); // no subsampling applied
    write_byte_buffer(b, 0x00); // quantization table id
}

void write_dht(buffer_t *b)
{
    int i;

    write_2bytes_buffer(b, DHT_MARKER);
    write_2bytes_buffer(b, sum_dc_val + 19);
    write_byte_buffer(b, 0x00);
    //first huffman table (for dc values)
    for(i=0; i<16; i++)
        write_byte_buffer(b, dc_num[i+1]);
    for(i=0; i<sum_dc_val; i++)
        write_byte_buffer(b, dc_category[i]);
    //second huffman tables (for AC values)
    write_2bytes_buffer(b, DHT_MARKER);
    write_2bytes_buffer(b, sum_ac_val + 19);
    write_byte_buffer(b, 0x10);
    for(i=0; i<16; i++)
        write_byte_buffer(b, ac_num[i+1]);
    for(i=0; i<sum_ac_val; i++)
        write_byte_buffer(b, ac_category[i]);
}

void write_sos(buffer_t *b)
{
    write_2bytes_buffer(b, SOS_MARKER); //Start Of Scan marker
    write_2bytes_buffer(b, 8); //length
    write_byte_buffer(b, 0x01); // nb. components
    write_byte_buffer(b, 0x01); // Y component ID
    write_byte_buffer(b, 0x00);     // Y HT = 0 
    write_byte_buffer(b, 0x00);
    write_byte_buffer(b, 0x3F);
    write_byte_buffer(b, 0x00);
}

void write_eoi(buffer_t *b)
{
    write_2bytes_buffer(b, EOI_MARKER); //Start Of Image marker
}

void write_bitstream(buffer_t *b, int **rle_Y, int no_8x8_blocks, int width, int height)
{
    unsigned char q_array[64], value;
    unsigned char buf[1];
    int value_DC[11], value_AC[11], code_DC[9], code_AC[16], final_array[8], buffer[128];
    int bits, code_length, count, number, i, j, k, l = 0;
    int value_bits = -1, code_bits = -1;

    write_soi(b);
    write_app0(b);
    write_dqt(b, jpec_qzr);
    write_sof(b, width, height);
    write_dht(b);
    write_sos(b);
    
    init_array(final_array, 8);
    init_array(buffer, 128);

   for(i=0; i<no_8x8_blocks; i++)
    {
        init_array(value_DC, 11);
        init_array(code_DC, 9);

        encode_DC_value(rle_Y[i][0], value_DC, code_DC, &value_bits, &code_bits);
        flush_bits(code_DC, value_DC, final_array, buffer, value_bits, code_bits);

        //insert code_DC and value_DC into bitstream
        if(is_byte(final_array, 8))
        {
            value = convert_binary_to_dec(final_array, 8);
            if(value == 255)
            {
                write_byte_buffer(b, value);
                write_byte_buffer(b, 0x00);
            }
            else
                write_byte_buffer(b, value);
            init_array(final_array, 8);
        }
        for(j=1; j<72; j++)
        {
            init_array(value_AC, 11);
            init_array(code_AC, 16);

            if(rle_Y[i][j] == 0)
            {
                if(rle_Y[i][j+1] != 0 && rle_Y[i][j+1]!=400)
                {
                    if(rle_Y[i][j+1] > 16)
                    {
                       do{
                            encode_AC_value(0, 16, value_AC, code_AC, &value_bits, &code_bits);
                            flush_bits(code_AC, value_AC, final_array, buffer, 0, code_bits);
                            rle_Y[i][j+1] -=16;
                        
                        }while(rle_Y[i][j+1]>16);

                        if(is_byte(final_array, 8))
                        {
                            value = convert_binary_to_dec(final_array, 8);
                            if(value == 255)
                            {
                                write_byte_buffer(b, value);
                                write_byte_buffer(b, 0x00);
                            }
                            else
                                write_byte_buffer(b, value);
                            init_array(final_array, 8);
                        }
                    }
                    if(rle_Y[i][j+1] == 16)
                    {
                        encode_AC_value(0, 16, value_AC, code_AC, &value_bits, &code_bits);
                        flush_bits(code_AC, value_AC, final_array, buffer, 0, code_bits);

                        if(is_byte(final_array, 8))
                        {
                            value = convert_binary_to_dec(final_array, 8);
                            if(value == 255)
                            {
                                write_byte_buffer(b, value);
                                write_byte_buffer(b, 0x00);
                            }
                            else
                                write_byte_buffer(b, value);
                            init_array(final_array, 8);
                        }
                    }
                    else
                    {
                        encode_AC_value(rle_Y[i][j+2], rle_Y[i][j+1], value_AC, code_AC, &value_bits, &code_bits);
                        flush_bits(code_AC, value_AC, final_array, buffer, value_bits, code_bits);

                        if(is_byte(final_array, 8))
                        {
                            value = convert_binary_to_dec(final_array, 8);
                            if(value == 255)
                            {
                                write_byte_buffer(b, value);
                                write_byte_buffer(b, 0x00);
                            }
                            else
                                write_byte_buffer(b, value);
                        init_array(final_array, 8);
                        }
                    }
                    j+=2; 

                }
                else
                {
                    //EOB
                    encode_AC_value(rle_Y[i][j], 0, value_AC, code_AC, &value_bits, &code_bits);
                    flush_bits(code_AC, value_AC, final_array, buffer, value_bits, code_bits);
                    
                    if(is_byte(final_array, 8))
                    {
                        value = convert_binary_to_dec(final_array, 8);
                        if(value == 255)
                        {
                            write_byte_buffer(b, value);
                            write_byte_buffer(b, 0x00);
                        }
                        else
                            write_byte_buffer(b, value);
                        init_array(final_array, 8);
                    }
                    j = 72;
                }
            }
            else if(rle_Y[i][j]==INT_MAX)
            {
                j = 72;
            }
            else
            {
                encode_AC_value(rle_Y[i][j], 0, value_AC, code_AC, &value_bits, &code_bits);
                flush_bits(code_AC, value_AC, final_array, buffer, value_bits, code_bits);
                
                if(is_byte(final_array, 8))
                {
                    value = convert_binary_to_dec(final_array, 8);
                    if(value == 255)
                        {
                            write_byte_buffer(b, value);
                            write_byte_buffer(b, 0x00);
                        }
                        else
                            write_byte_buffer(b, value);
                    init_array(final_array, 8);
                }
            } 
        }
    }    
    for(k=0; k<8; k++)
    {
        if(buffer[k] != -1)
        {
            final_array[l] = buffer[k];
            l++;
        } 
    }

    for(k=0; k<8; k++)
    {
        if(k>0 && final_array[k] == -1)
            final_array[k] = 1;
    }
    if(is_byte(final_array, 8))
    {
        value = convert_binary_to_dec(final_array, 8);
        write_byte_buffer(b, value);
    }
    write_2bytes_buffer(b, EOI_MARKER);
}

