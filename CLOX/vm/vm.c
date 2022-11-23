#include <stdio.h>
#include "../common.h"
#include "vm.h"
#include "../disassemmbler/disassemble.h"

VM vm;

void initVM() {

} 

void freeVM() {

} 

InterpretResult interpret(Chunk* chunk) {
	vm.chunk = chunk;
	vm.ip = vm.chunk->code;
	return run();
} 

 static InterpretResult run() {

	#define READ_BYTE() (*vm.ip++)
	#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()]) 
	// For constants, the chunk->code stores the constant's index
	// Which is why read_byte would return an index which we can later use to get the 
	// value from the values array 

	for (;;) {
		
		#ifdef DEBUG_TRACE_EXECUTION
		int testing = *vm.ip; // shows int becuase it represents the enum values
		int testing2 = vm.ip - vm.chunk->code; // subtract the current address by the starting address
		disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
		#endif

		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
			case OP_CONSTANT: {
				Value constant = READ_CONSTANT(); 
				printValue(constant);
				printf("\n");
				break;
			}
			case OP_RETURN: {
				return INTERPRET_OK;
			}
		}
	} 

	#undef READ_BYTE
	#undef READ_CONSTANT

}