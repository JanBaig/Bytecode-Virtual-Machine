#ifndef clox_vm_h
#define clox_vm_h

#include "../chunk//chunk.h"
#include "../value/value.h"

#define STACK_MAX 256

typedef struct {
	Chunk* chunk;
	uint8_t* ip; // points to the next instruction, not the one currently being handled
	Value* stack;
	int stackCapacity;
	int stackCount; // points to where the NEXT value should go
	Obj* objects;
} VM;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif 
