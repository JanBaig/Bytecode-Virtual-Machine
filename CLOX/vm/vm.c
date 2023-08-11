#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>

#include "../common.h"
#include "vm.h"
#include "../disassemmbler/disassemble.h"
#include "../compiler/compiler.h"
#include "../memory/memory.h"
#include "../objects/objects.h"

VM vm;
bool foundConstantLong = false;

static void resetStack() {
	vm.stackCount = 0; // indicates that stack is now empty
}

static void runtimeError(const char* format, ...) {
	va_list args; 
	va_start(args, format);
	vfprintf(stderr, format, args); // writes the arguments to the stderr stream
	va_end(args);
	fputs("\n", stderr);

	size_t instructionIndex = vm.ip - vm.chunk->code - 1; // -1 since .ip points to the NEXT instruction 
	int line = getLine(&vm.chunk, instructionIndex);
	fprintf(stderr, "[line %d] in script\n", line); 
	resetStack();
}

void initVM() {
	vm.stack = NULL;
	vm.stackCapacity = 0;
	resetStack();
	vm.objects = NULL;
} 

void freeVM() {
	// Free the dynamic stack array
	FREE_ARRAY(Value, vm.stack, vm.stackCapacity);
	freeObjects();
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

static Value peek(int distance) {
	// Subtracting 1 because stackCount keeps track of the next EMPTY slot in the stack
	return vm.stack[vm.stackCount -1 - distance]; 
}

static bool isFalsey(Value value) {
	// NIL and false are falsey and every other value is true
	return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
	ObjString* b = AS_STRING(pop());
	ObjString* a = AS_STRING(pop());

	int length = a->length + b->length;
	char* chars = ALLOCATE(char, length + 1);
	memcpy(chars, a->chars, a->length);
	memcpy(chars + a->length, b->chars, b->length);
	chars[length] = '\0';

	ObjString* result = takeString(chars, length);
	push(OBJ_VAL(result));
}

InterpretResult interpret(const char* source) {
	Chunk chunk;
	initChunk(&chunk);

	// If chunk does not compile into bytecode without errors (SCANNER + COMPILER)
	if (!compile(source, &chunk)) {
		freeChunk(&chunk);
		return INTERPRET_COMPILE_ERROR;
	} 

	// If no compilation error, we start the interpretation process (VM)
	vm.chunk = &chunk;
	vm.ip = vm.chunk->code;

	InterpretResult result = run();

	freeChunk(&chunk);
	return result;
} 

static void testStack(bool boolean) {
	
	struct timespec begin;
	timespec_get(&begin, TIME_UTC);

	if (boolean) { push(NUMBER_VAL(-AS_NUMBER(pop()))); }
	else { vm.stack[vm.stackCount - 1] = NUMBER_VAL(- AS_NUMBER(vm.stack[vm.stackCount - 1])); }

	struct timespec end;
	timespec_get(&end, TIME_UTC);

	double time_spent = (end.tv_nsec - begin.tv_nsec);
	printf("RESULT: %f\n", time_spent);
}

static InterpretResult run() {

	#define READ_BYTE() (*vm.ip++) // returns an enum value (int)
	#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()]) 
	#define BINARY_OP(valueType, op) \
			do { \
				if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
					runtimeError("Operands must be numbers."); \
					return INTERPRET_RUNTIME_ERROR; \
				} \
				double b = AS_NUMBER(pop()); \
				double a = AS_NUMBER(pop()); \
				push(valueType(a op b)); \
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
			case OP_NIL: push(NIL_VAL); break;
			case OP_TRUE: push(BOOL_VAL(true)); break;
			case OP_FALSE: push(BOOL_VAL(false)); break;
			case OP_EQUAL: {
				Value b = pop();
				Value a = pop();
				push(BOOL_VAL(valuesEqual(a, b)));
				break;
			}
			case OP_GREATER:  BINARY_OP(BOOL_VAL, > ); break;
			case OP_LESS:     BINARY_OP(BOOL_VAL, < ); break;
			case OP_CONSTANT_LONG: {
				foundConstantLong = true;
  				Value constant = READ_CONSTANT();
				push(constant);
				break;
			}
			case OP_ADD: {
				if (IS_STRING(peek(0)) && IS_STRING(peek(1))) { 
					concatenate(); 
				} else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
					double b = AS_NUMBER(pop());
					double a = AS_NUMBER(pop());
					push(NUMBER_VAL(a + b));
				} else {
					runtimeError("Operands must be two numbers or two strings.");
					return INTERPRET_RUNTIME_ERROR;
				} 
				break;
			}
			case OP_SUBTRACT:	BINARY_OP(NUMBER_VAL, -); break;
			case OP_MULTIPLY:	BINARY_OP(NUMBER_VAL, *); break;
			case OP_DIVIDE:		BINARY_OP(NUMBER_VAL, /); break;
			case OP_NOT: push(BOOL_VAL(isFalsey(pop()))); break;
			case OP_NEGATE: {
				if (!IS_NUMBER(peek(0))) {
					runtimeError("Operand must be a number.");
					return INTERPRET_RUNTIME_ERROR;
				}
				vm.stack[vm.stackCount - 1] = NUMBER_VAL(- AS_NUMBER(vm.stack[vm.stackCount - 1]));
				break;
			}
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

