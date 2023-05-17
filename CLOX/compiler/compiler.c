#include <stdio.h>
#include <stdlib.h>

#include "../common.h"
#include "compiler.h"
#include "../scanner/scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "../disassemmbler/disassemble.h"
#endif

typedef struct {
	Token current;
	Token previous;
	bool hadError;
	bool panicMode;
} Parser;

typedef enum {
	PREC_NONE,		  // 0
	PREC_ASSIGNMENT,  // 1 =
	PREC_OR,          // 2 or
	PREC_AND,         // 3 and
	PREC_EQUALITY,    // 4 == !=
	PREC_COMPARISON,  // 5 < > <= >=
	PREC_TERM,        // 6 + -
	PREC_FACTOR,      // 7 * /
	PREC_UNARY,       // 8 ! -
	PREC_CALL,        // 9 . ()
	PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();

typedef struct { 
	// Represents a single row in the parsing table
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

Parser parser; 
Chunk* compilingChunk; 

static Chunk* currentChunk() {
	return compilingChunk;
}

static void errorAt(Token* token, const char* message) {
	if (parser.panicMode) return; // don't want to spew the rest of the errors and have an error cascade
	parser.panicMode = true;
	fprintf(stderr, "[line %d] Error", token->line);

	if (token->type == TOKEN_EOF) { // if the prev token was TOKEN_ERROR and the current one is TOKEN_EOF
		fprintf(stderr, " at end");
	}
	else if (token->type == TOKEN_ERROR) {
		// Nothing
	}
	else {
		fprintf(stderr, " at '%.*s'", token->length, token->start); // consumed token was not of expected type
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
	// CLOX's scanner leaves an error token (if error in source) and leaves it up to the parser to handle it
	// The parser asks the scanner repeatedly for the next token in the loop

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

static uint8_t makeConstant(Value value) {
	int constantIndex = addConstant(currentChunk(), value);
	if (constantIndex > UINT8_MAX) {
		error("Too many constants in one chunk.");
		return 0;
	} 

	return (uint8_t)constantIndex; // index returned takes up 1 byte in space - "bytecode"
}

static void emitConstant(Value value) {
	// Adds Instruction + Index (Byte sized) to the code array in the chunk
	// Floating point value is added to the chunk's constants attribute
	emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler() {
	emitReturn(); 

	#ifdef DEBUG_PRINT_CODE 
	if (!parser.hadError) {
		disassembleChunk(currentChunk(), "code");
	} 

	#endif
}

static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static void binary() {
	// Infix operator has already been consumed - which is why we use the previous token
	TokenType operatorType = parser.previous.type;
	ParseRule* rule = getRule(operatorType);
	parsePrecedence((Precedence)(rule->precedence + 1));

	switch (operatorType) {
		case TOKEN_PLUS: emitByte(OP_ADD); break;
		case TOKEN_MINUS: emitByte(OP_SUBTRACT); break;
		case TOKEN_STAR: emitByte(OP_MULTIPLY); break;
		case TOKEN_SLASH: emitByte(OP_DIVIDE); break;
		default: return; // Unreachable.
	}
}

static void grouping() {
	// We assume the '(' has already been consumed as that is what initially
	// invokes this function in the first place
	expression(); 
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

static void number() { 
	// string -> double conversion
	double value = strtod(parser.previous.start, NULL);
	emitConstant(value);
}

static void unary() {
	TokenType operatorType = parser.previous.type;

	// compile the operand
	parsePrecedence(PREC_UNARY);

	// Emit the operator Instruction
	switch (operatorType) {
		case TOKEN_MINUS: emitByte(OP_NEGATE); break;
		default: return;
	}
} 

ParseRule rules[] = {
  [TOKEN_LEFT_PAREN] = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RIGHT_BRACE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS] = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS] = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH] = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR] = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER] = {number,   NULL,   PREC_NONE},
  [TOKEN_AND] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FOR] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_OR] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_VAR] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF] = {NULL,     NULL,   PREC_NONE},
};

static void parsePrecedence(Precedence precedence) {
	advance();
	ParseFn prefixRule = getRule(parser.previous.type)->prefix;

	if (prefixRule == NULL) {
		error("Expect expression");
		return;
	}

	prefixRule(); // parse in accordance to the token type

	while (precedence <= getRule(parser.current.type)->precedence) {
		advance();
		ParseFn infixRule = getRule(parser.previous.type)->infix;
		infixRule();
	} 
}

static ParseRule* getRule(TokenType type) {
	// returns the address of the rule in ParseRule
	return &rules[type];
}

static void expression() {
	// We simply parse the lowest precedence level
	// which subsumes all of the higher-precedence expressions too
	parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char* source, Chunk* chunk) { 
	initScanner(source);
	compilingChunk = chunk;

	parser.hadError = false;
	parser.panicMode = false;

	advance(); // Accounts for errors at the start - If contains an error, keeps on looping until only valid tokens are found
	expression();
	consume(TOKEN_EOF, "Expect end of expression.");
	endCompiler(); // emits the OP_RETURN bytecode instruction
	return !parser.hadError;
}  

