#include "common.h" 
#include "Chunk/chunk.h"
#include "./Disassemmbler/disassemble.h" 

int main(int  argc, const char* argv[]) {
	Chunk chunk;
	initChunk(&chunk);
	int constant = addConstant(&chunk, 1.2);
	writeChunk(&chunk, OP_CONSTANT, 123); // 1 byte op
	writeChunk(&chunk, constant, 123); // 1 byte operand, 2 byte instruction format
	writeChunk(&chunk, OP_RETURN, 123);
	
	disassembleChunk(&chunk, "Test Chunk");
	freeChunk(&chunk);

	return 0;
}  






