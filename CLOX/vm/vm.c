#include <stdio.h>
#include "../common.h"
#include "vm.h"
#include "../disassemmbler/disassemble.h"
#include "../compiler/compiler.h"
#include "../memory/memory.h"

VM vm;
bool foundConstantLong = false;

static void resetStack() {
	vm.stackCount = 0; // indicates that stack is now empty
}

void initVM() {
	vm.stack = NULL;
	vm.stackCapacity = 0;
	resetStack();
} 

void freeVM() {
	// Free the dynnamic stack array
	FREE_ARRAY(Value, vm.stack, vm.stackCapacity);
}  

void push(Value value) {

	if (vm.stackCapacity < vm.stackCount + 1) { 
		int oldCapacity = vm.stackCapacity;
		vm.stackCapacity = GROW_CAPACITY(oldCapacity);
		vm.stack = GROW_ARRAY(Value, vm.stack, oldCapacity, vm.stackCapacity);
	}

	vm.stack[vm.stackCount] = value;
	vm.stackCount++;
} 
 
Value pop() {
	vm.stackCount--;
	return vm.stack[vm.stackCount];
}

InterpretResult interpret(const char* source) {
	Chunk chunk;
	initChunk(&chunk);

	// If chunk does not compile into bytecode without errors
	if (!compile(source, &chunk)) {
		freeChunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	} 

	// If no compilation error, we start the interpretation process
	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;

	InterpretResult result = run();

	freeChunk(&chunk);
	return result;
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
		for (Value* slot = vm.stack; slot < vm.stack + vm.stackCount; slot++) { // prints what is already present in the stack
			printf("[ ");
			printValue(*slot);
			printf(" ]");
		} 
		printf("\n");

		int offset = (int)(vm.ip - vm.chunk->code);
		int prevInstruc = vm.chunk->code[offset - 2];
		if (prevInstruc == OP_CONSTANT_LONG && foundConstantLong) {
			// Since the OP_CONSTANT_LONG has a 3 byte operand 
			// We've already read one of the operands so we skip the 2 to get to the next instruction
			vm.ip += 2; 
			foundConstantLong = false;
		}

		disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code)); // getting the offset
		#endif

		uint8_t instruction;
		switch (instruction = READ_BYTE()) {
			case OP_CONSTANT: {
				Value constant = READ_CONSTANT(); 
				push(constant);
				break;
			} 
			case OP_CONSTANT_LONG: {
				foundConstantLong = true;
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

