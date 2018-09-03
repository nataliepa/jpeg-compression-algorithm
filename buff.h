#ifndef BUFFER_H
#define BUFFER_H

typedef struct buffer
{
  uint8_t *stream;  //byte buffer
  int length;       //current length
  int size;         //initial size
  int n_bits;       //maximum size
} buffer_t;

//initilize buffer array
buffer_t *init_buffer(int size);

//write one byte into buffer
void write_byte_buffer(buffer_t *b, int value);

//wite two bytes into buffer
void write_2bytes_buffer(buffer_t *b, int value);
#endif