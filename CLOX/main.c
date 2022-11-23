#include "common.h" 
#include "chunk/chunk.h"
#include "./disassemmbler/disassemble.h"
#include "./vm/vm.h"

int main(int  argc, const char* argv[]) {
	
	initVM();
	
	Chunk chunk;
	initChunk(&chunk);
	int constantIndex = addConstant(&chunk, 1.2);
	writeChunk(&chunk, OP_CONSTANT, 123); // 
	writeChunk(&chunk, constantIndex, 123); 
	writeChunk(&chunk, OP_RETURN, 123);
	
	// disassembleChunk(&chunk, "Test Chunk");
	
	interpret(&chunk);

	freeVM();
	freeChunk(&chunk);

	return 0;
}  






