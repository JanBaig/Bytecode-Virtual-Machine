#ifndef clox_chunk_h 
#define clox_chunk_h

#include "../common.h"

typedef enum {
	OP_RETURN, 
} OpCode;

typedef struct { 
	int count;
	int capacity;
	uint8_t* code;  // code contains address of a variable that is a dynamic array and contains byte sized ints
} Chunk; 

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte);
void freeChunk(Chunk* chunk);

#endif





