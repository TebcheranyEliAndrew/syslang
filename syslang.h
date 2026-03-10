#ifndef SYSLANG_H
#define SYSLANG_H

#include <stdbool.h>

// Token types - These are the "words" your language understands
typedef enum {
    TOKEN_PRINT,        // The keyword "print"
    TOKEN_IDENTIFIER,   // Variable names like "cpu" or "ram"
    TOKEN_STRING,       // Text in quotes like "Hello"
    TOKEN_NUMBER,       // Numbers like 42 or 1024
    TOKEN_PLUS,         // The + operator for concatenation
    TOKEN_EQUALS,       // The = operator for assignment
    TOKEN_LPAREN,       // Left parenthesis (
    TOKEN_RPAREN,       // Right parenthesis )
    TOKEN_EOF,          // End of file - no more tokens
    TOKEN_ERROR         // Something went wrong
} SysLangTokenType;

typedef struct {
    SysLangTokenType type;
    char* value;
    int line;
} Token;

// AST node types - These represent the STRUCTURE of your code
typedef enum {
    NODE_PRINT,         // A print statement
    NODE_ASSIGN,        // An assignment like cpu = "Intel"
    NODE_STRING,        // A string literal like "Hello"
    NODE_IDENTIFIER,    // A variable name like cpu
    NODE_FUNC_CALL,     // A function call like get_cpu_info()
    NODE_BINOP          // A binary operation like "a" + "b"
} NodeType;

// One node in the Abstract Syntax Tree
typedef struct ASTNode {
    NodeType type;      // What kind of node?
    
    // Union means only ONE of these is used, depending on type
    union {
        // For NODE_PRINT: print something
        struct { struct ASTNode* expr; } print;
        
        // For NODE_ASSIGN: assign a value to a variable
        struct { char* name; struct ASTNode* value; } assign;
        
        // For NODE_STRING: a string value
        struct { char* value; } string;
        
        // For NODE_IDENTIFIER: a variable name
        struct { char* name; } identifier;
        
        // For NODE_FUNC_CALL: calling a function with arguments
        struct { 
            char* name;           // Function name like "get_cpu_info"
            struct ASTNode** args; // Array of arguments
            int arg_count;         // How many arguments
        } call;
        
        // For NODE_BINOP: left + right operation
        struct { 
            struct ASTNode* left;   // Left side
            struct ASTNode* right;  // Right side
            char op;                // Operator like '+'
        } binop;
    } data;
} ASTNode;

// Interpreter state - keeps track of variables
typedef struct {
    char** var_names;    // Array of variable names (like "cpu")
    char** var_values;   // Array of variable values (like "Intel i7")
    int var_count;       // How many variables we have
    int var_capacity;    // How many we can store (for resizing)
} Interpreter;

// Public API - these are the functions you'll call from main.c
Token* lex(const char* source, int* count);           // Convert text to tokens
ASTNode* parse(Token* tokens, int count);              // Convert tokens to AST
char* interpret(Interpreter* interp, ASTNode* node);   // Execute the AST
void free_ast(ASTNode* node);                          // Clean up memory
void free_tokens(Token* tokens, int count);            // Clean up memory

// Hardware functions - implemented per operating system
const char* get_cpu_info(void);
unsigned long long get_ram_total(void);
unsigned long long get_ram_used(void);
const char* get_os_name(void);

#endif