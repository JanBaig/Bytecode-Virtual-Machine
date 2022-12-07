#include <stdio.h>
#include "../common.h"
#include "vm.h"
#include "../disassemmbler/disassemble.h"

VM vm;

static void resetStack() {
	vm.stackTop = vm.stack; // indicates that stack is now empty
}

void initVM() {
	resetStack();
} 

void freeVM() {
	
}  

void push(Value value) {
	*vm.stackTop = value;
	vm.stackTop++;
}

Value pop() {
	vm.stackTop--;
	return *vm.stackTop;
}

InterpretResult interpret(Chunk* chunk) {
	vm.chunk = chunk;
	vm.ip = vm.chunk->code;
	return run();
} 

 static InterpretResult run() {

	#define READ_BYTE() (*vm.ip++) // returns an enum value (int)
	#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()]) 
	#define BINARY_OP(op) \
			do { \
				double b = pop(); \
				double a = pop(); \
				push (a op b); \
			} while (false);

	for (;;) {
		
		#ifdef DEBUG_TRACE_EXECUTION
		printf("		");
		for (Value* slot = vm.stack; slot < vm.stackTop; slot++) { // prints what is already present in the stack
			printf("[ ");
			printValue(*slot);
			printf(" ]");
		} 
		printf("\n");

		disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code)); // getting the offset
		#endif

		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
			case OP_CONSTANT: {
				Value constant = READ_CONSTANT(); 
				push(constant);
				break;
			}
			case OP_ADD: BINARY_OP(+); break;
			case OP_SUBTRACT: BINARY_OP(-); break;
			case OP_MULTIPLY: BINARY_OP(*); break;
			case OP_DIVIDE: BINARY_OP(/ ); break;
			case OP_NEGATE: push(-pop()); break;
			case OP_RETURN: {
				printValue(pop());
				printf("\n");
				return INTERPRET_OK;
			}
		}
	} 

	#undef READ_BYTE
	#undef READ_CONSTANT
	#undef BINARY_OP

} 








