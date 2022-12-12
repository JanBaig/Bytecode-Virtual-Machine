#include <stdio.h>
#include <stdlib.h>

#include "../common.h"
#include "compiler.h"
#include "../scanner/scanner.h"

typedef struct {
	Token current;
	Token previous;
	bool hadError;
	bool panicMode;
} Parser;

Parser parser; 
Chunk* compilingChunk; 

static Chunk* currentChunk() {
	return compilingChunk;
}

static void errorAt(Token* token, const char* message) {
	if (parser.panicMode) return; // don't want to spew the rest of the errors, do one at a time instead
	parser.panicMode = true;
	fprintf(stderr, "[line %d] Error", token->line);

	if (token->type == TOKEN_EOF) {
		fprintf(stderr, " at end");
	}
	else if (token->type == TOKEN_ERROR) {
		// Nothing
	}
	else {
		fprintf(stderr, " at '%.*s'", token->length, token->start);
	} 

	fprintf(stderr, ": %s\n", message);
	parser.hadError = true;
}

static void error(const char* message) {
	errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message) {
	errorAt(&parser.current, message);
}

static void advance() {
	// CLOX's scanner includes an error token and leaves it up to the parser to handle 
	// The parser asks the scanner repetitively for the next token in the loop

	parser.previous = parser.current; 

	for (;;) {
		parser.current = scanToken();
		if (parser.current.type != TOKEN_ERROR) break; 
		errorAtCurrent(parser.current.start);
	}
}

static void consume(TokenType type, const char* message) {
	if (parser.current.type == type) {
		advance();
		return;
	} 

	errorAtCurrent(message);
} 

static void emitByte(uint8_t byte) {
	writeChunk(currentChunk(), byte, parser.previous.line);
} 

static void emitBytes(uint8_t byte1, uint8_t byte2) {
	// Defined for convenience when we need to write an opcode and its operand together
	emitByte(byte1);
	emitByte(byte2);
}

static void emitReturn() {
	emitByte(OP_RETURN);
}

static void endCompiler() {
	emitReturn();
}

static void expression() {
	// What goes here?
}

bool compile(const char* source, Chunk* chunk) {
	initScanner(source);
	compilingChunk = chunk; // compiling chunk holds the address of the original chunk, so same thing (until we get to functions and scope)

	parser.hadError = false;
	parser.panicMode = false;

	advance();
	expression();
	consume(TOKEN_EOF, "Expect end of expression.");
	endCompiler();
	return !parser.hadError;
}  

