#include "common.h" 
#include "Chunk/chunk.h"
#include "Debug/debug.h" 

int main(int  argc, const char* argv[]) {
	Chunk chunk;
	initChunk(&chunk);
	writeChunk(&chunk, OP_RETURN);
	
	disassembleChunk(&chunk, "Test Chunk");

	freeChunk(&chunk);

	return 0;
}  






