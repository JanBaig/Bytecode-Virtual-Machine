#include <stdlib.h>
#include "../chunk/chunk.h"
#include "../memory/memory.h"

void initChunk(Chunk* chunk) {
	chunk->count = 0;
	chunk->capacity = 0;
	chunk->code = NULL; 
	initValueArray(&chunk->constants);

	chunk->lines = NULL;
	chunk->linesCapacity = 0;
	chunk->linesCount = 0;
} 

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
	
	// Updating Bytecode Array Data
	if (chunk->capacity < chunk->count + 1) {
		int oldCapacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(oldCapacity);
		chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
	}

	// Updating Line Data
	if (line > chunk->linesCount) {
		
		if (chunk->linesCapacity <  chunk->linesCount + 1) {
			int oldCapacity = chunk->linesCapacity;
			chunk->linesCapacity = GROW_CAPACITY(oldCapacity);
			chunk->lines = GROW_ARRAY(int*, chunk->lines, oldCapacity, chunk->linesCapacity);
		} 

		// New Line into 2D Line Array
		chunk->lines[chunk->linesCount] = NULL;
		chunk->lines[chunk->linesCount] = GROW_ARRAY(int, chunk->lines[chunk->linesCount], 0, 2);
		chunk->lines[chunk->linesCount][0] = 1;
		chunk->lines[chunk->linesCount][1] = line; 
		chunk->linesCount++;
	}
	else { chunk->lines[chunk->linesCount-1][0]++; }

	chunk->code[chunk->count] = byte;
	chunk->count++;
} 

void freeChunk(Chunk* chunk) {
	FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
	freeValueArray(&chunk->constants);

	// Print the 2D array First
	for (int i = 0; i < chunk->linesCount; i++) {
		printf("(Count: %d, Line %d)\n", chunk->lines[i][0], chunk->lines[i][1]);
	}

	// Free 2D Line Array - Print the values for testing first (?)
	for (int i = 0; i < chunk->linesCount; i++) { FREE_ARRAY(int, chunk->lines[i], 2); }
	FREE_ARRAY(int*, chunk->lines, chunk->linesCapacity);

	initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value) {
	// chunk->constants = ValueArray
	// ValueArray->values, count
	
	writeValueArray(&chunk->constants, value);
	// -1 is required because count holds the # of values and is not 0-indexed
	// which is what we need for indexing the ValueArray (used in our bytecode instruction's operand)
	return chunk->constants.count - 1; 
	
}

int getLine(Chunk* chunk, int byteIndex) {
	
	for (int i = 0; i < chunk->linesCount; i++) {
		if (byteIndex < chunk->lines[i][0]) {
			return chunk->lines[i][1];
		}
	}
	return 0;
} 
