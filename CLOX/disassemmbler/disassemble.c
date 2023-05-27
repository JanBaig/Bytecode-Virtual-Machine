#include <stdio.h>
#include "./disassemble.h"
#include "../value/value.h"
#include "../chunk/chunk.h"

void disassembleChunk(Chunk* chunk, const char* name) {
	printf("== %s ==\n", name);
	
	for (int offset = 0; offset < chunk->count;) { 
		offset = disassembleInstruction(chunk, offset);
	}
}

int disassembleInstruction(Chunk* chunk, int offset) {
	printf("%04d ", offset);

	if (offset > 0 && getLine(chunk, offset) == getLine(chunk, offset - 1)) {
		printf("	| ");
	}
	else {
		printf("%4d ", getLine(chunk, offset));
	}
	uint8_t instruction = chunk->code[offset];
	switch (instruction) {
		case OP_CONSTANT:
			return constantInstruction("OP_CONSTANT", chunk, offset);
		case OP_CONSTANT_LONG:
			return constantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
		case OP_ADD: 
			return simpleInstruction("OP_ADD", offset);
		case OP_SUBTRACT: 
			return simpleInstruction("OP_SUBTRACT", offset);
		case OP_MULTIPLY:
			return simpleInstruction("OP_MULTIPLY", offset);
		case OP_DIVIDE:
			return simpleInstruction("OP_DIVIDE", offset);
		case OP_NEGATE: 
			return simpleInstruction("OP_NEGATAE", offset);
		case OP_RETURN:
			return simpleInstruction("OP_RETURN", offset);
		default:
			printf("Unknown opcode %d\n", instruction); 
			return offset + 1;
	}
} 

static int simpleInstruction(const char* name, int offset) {
	printf("%s\n", name);
	return offset + 1;
} 

static int constantInstruction(const char* name, Chunk* chunk, int offset ) {
	uint8_t constantIndex = chunk->code[offset + 1]; // index 0 is the opcode and +1 is the operand (which is an index)
	printf("%-16s %4d '", name, constantIndex);
	printValue(chunk->constants.values[constantIndex]);
	printf("'\n");
	return offset + 2;
}

static int binaryToDec(int binary) {
	
	int decimal_num = 0;
	int base = 1;
	int rem;

	while (binary > 0) {
		rem = binary % 10;
		decimal_num = decimal_num + rem * base;
		binary /= 10;
		base *= 2;
	} 

	return decimal_num;
}

static int constantLongInstruction(const char* name, Chunk* chunk, int offset) {
	
	// Getting the 3 byte index
	uint8_t byteArr[3];
	byteArr[0] = chunk->code[offset + 1]; 
	byteArr[1] = chunk->code[offset + 2];
	byteArr[2] = chunk->code[offset + 3]; 
	
	// Converting the 3 byte index into binary
	char buffer1[9], buffer2[9], buffer3[9]; // 9 = 8 bit + 1
	char dest[28]; // 28 = (9 + 9 + 9) + 1

	_itoa_s(byteArr[2], buffer3, 9, 2);
	_itoa_s(byteArr[1], buffer2, 9, 2);
	_itoa_s(byteArr[0], buffer1, 9, 2);

	// Concatenate the binary values and converting to a decimal integer (the index)
 	strcpy_s(dest, 28, buffer3);
	strcat_s(dest, 28, buffer2);
	strcat_s(dest, 28, buffer1);
	
	int constantIndex = binaryToDec(atoi(dest));

	printf("%-16s %4d '", name, constantIndex);
	printValue(chunk->constants.values[constantIndex]);
	printf("'\n");
	return offset + 4;
}