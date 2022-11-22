#ifndef clox_chunk_h 
#define clox_chunk_h

#include "../common.h"
#include "../Value/value.h"

typedef enum {
	OP_CONSTANT,
	OP_RETURN, 
} OpCode;

typedef struct { 
	int count;
	int capacity;
	ValueArray constants;
	uint8_t* code; 
	int* lines;
} Chunk; 

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value);

#endif

// code contains address of a variable that is a dynamic array and contains byte sized ints
