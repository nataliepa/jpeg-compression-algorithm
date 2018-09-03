#ifndef JPEG_FILE_H
#define JPEG_FILE_H

#include "buff.h"

//write Start Of Image(SOI) marker
void write_soi(buffer_t *b);
//write APP0 marker
void write_app0(buffer_t *b);
//write Quantization tables
void write_dqt(buffer_t *b, uint8_t array[]);
//write Start Of Frame (SOF) marker
void write_sof(buffer_t *b, uint16_t width, uint16_t height);
//write Huffman tables
void write_dht(buffer_t *b);
//write Start Of Scan marker
void write_sos(buffer_t *b);
//write End Of Image (EOI) marker 
void write_eoi(buffer_t *b);
//write bitstream
void write_bitstream(buffer_t *b, int **rle_Y, int no_8x8_blocks, int width, int height);
#endif