#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h" 
#include "chunk/chunk.h"
#include "./disassemmbler/disassemble.h"
#include "./vm/vm.h"

static void repl() {
	char line[1024]; // arrays decay into pointers by the compiler
	for (;;) {
		printf("> "); 

		if (!fgets(line, sizeof(line), stdin)) {
			printf("\n");
			break;
		}
		
		interpret(line);
	}
} 

static char* readFile(const char* path) {
	// 'b' opens in binary mode and disables handling of newlines
	FILE* file = fopen(path, "rb"); 
	if (file == NULL) {
		fprintf(stderr, "Could not open file \"%s\".\n", path);
		exit(74);
	}

	// Find the file size
	fseek(file, 0L, SEEK_END); 
	size_t fileSize = ftell(file);
	rewind(file); 

	// Reads file contents to buffer
	char* buffer = (char*)malloc(fileSize + 1); // + 1 for the null terminator
	if (buffer == NULL) {
		fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
		exit(74);
	} 

	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
	if (bytesRead < fileSize) {
		fprintf(stderr, "Could not read file \"%s\".\n", path);
		exit(74);
	}
		
	buffer[bytesRead] = '\0'; // string needs a null terminator

	fclose(file);
	return buffer;

}

static void runFile(const char* path) {
	char* source = readFile(path); // dynamically allocates the string
	InterpretResult result = interpret(source);
	free(source); 

	if (result == INTERPRET_COMPILE_ERROR) exit(65);
	if (result == INTERPRET_RUNTIME_ERROR) exit(70);

}

int main(int argc, const char* argv[]) {

	initVM();
	
	if (argc == 1) {
		repl();
	}
	else if (argc == 2) {
		runFile(argv[1]);
	} 
	else {
		fprintf(stderr, "Usage: clox [path]\n"); // not buffered so displayed immediatly
		exit(64);
	}

	freeVM();

	return 0;
}  






