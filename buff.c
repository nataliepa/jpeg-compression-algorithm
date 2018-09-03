#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "buff.h"

#define BUFFER_INIT_SIZE 65536

//initilize buffer array
buffer_t *init_buffer(int size)
{
    buffer_t *b = malloc(sizeof(buffer_t));
    
    b->stream = malloc(size * sizeof(uint8_t));
    b->size = size;
    b->length = 0;
    b->n_bits = 0;

    return b;
}

//write one byte into buffer
void write_byte_buffer(buffer_t *b, int value)
{
    if (b->size == b->length) 
    {
    int nsiz = (b->size > 0) ? 2 * b->size : BUFFER_INIT_SIZE;
    void* tmp = realloc(b->stream, nsiz);
    b->stream = (uint8_t *) tmp;
    b->size = nsiz;
    }
    b->stream[b->length++] = (uint8_t) value;
}

//wite two bytes into buffer
void write_2bytes_buffer(buffer_t *b, int value) 
{
    write_byte_buffer(b, (value >> 8) & 0xFF);
    write_byte_buffer(b, value & 0xFF);  
}

