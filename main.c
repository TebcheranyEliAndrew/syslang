#include "syslang.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    Interpreter interp = {0};  // Zero-initialize the interpreter

    if (argc > 1) {
        // Run a script file
        FILE* f = fopen(argv[1], "r");
        if (!f) {
            printf("Error: Cannot open %s\n", argv[1]);
            return 1;
        }
        
        // Read entire file
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        char* source = malloc(size + 1);
        fread(source, 1, size, f);
        source[size] = '\0';
        fclose(f);

        // Lex, parse, and interpret
        int token_count;
        Token* tokens = lex(source, &token_count);
        ASTNode* ast = parse(tokens, token_count);
        interpret(&interp, ast);
        
        // Clean up
        free(source);
        free_ast(ast);
        free_tokens(tokens, token_count);

    } else {
        // REPL (Read-Eval-Print Loop) mode
        printf("SysLang v0.1 - Type 'exit' to quit\n");
        char line[1024];  // Buffer for user input
        
        while (1) {
            printf("> ");  // Prompt
            if (!fgets(line, sizeof(line), stdin)) break;  // Read line
            
            if (strcmp(line, "exit\n") == 0) break;  // Exit command
            
            // Lex, parse, and interpret the line
            int token_count;
            Token* tokens = lex(line, &token_count);
            ASTNode* ast = parse(tokens, token_count);
            char* result = interpret(&interp, ast);

            // Print result if there is one
            if (result && strlen(result) > 0) {
                printf("%s\n", result);
            }
            
            // Clean up
            free(result);
            free_ast(ast);
            free_tokens(tokens, token_count);
        }
    }
    
    return 0;
}