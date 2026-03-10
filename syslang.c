#include "syslang.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Token* lex(const char* source, int* count) {
    // Allocate space for up to 1000 tokens (simple, no dynamic resizing)
    Token* tokens = malloc(sizeof(Token) * 1000);
    int pos = 0;           // Current position in source text
    int token_pos = 0;     // Current position in tokens array
    int line = 1;          // Current line number
    
    while (source[pos]) {  // Loop until end of string
        // Skip whitespace (spaces, tabs, newlines)
        if (isspace(source[pos])) {
            if (source[pos] == '\n') line++;  // Count lines
            pos++;                            // Move past whitespace
            continue;                          // Go to next character
        }

        // Handle strings in double quotes: "Hello World"
        if (source[pos] == '"') {
            pos++;  // Skip the opening quote
            int start = pos;  // Remember where the string starts
            
            // Keep reading until we find the closing quote
            while (source[pos] && source[pos] != '"') pos++;
            
            int len = pos - start;  // Length of the string
            char* str = malloc(len + 1);  // Allocate memory
            strncpy(str, source + start, len);  // Copy the string
            str[len] = '\0';  // Add null terminator
            
            // Create a token
            tokens[token_pos++] = (Token){TOKEN_STRING, str, line};
            
            if (source[pos] == '"') pos++;  // Skip closing quote
            continue;
        }

        // Handle identifiers (variable names) and keywords
        if (isalpha(source[pos])) {
            int start = pos;  // Remember start position
            
            // Keep reading while it's a letter, number, or underscore
            while (isalnum(source[pos]) || source[pos] == '_') pos++;
            
            int len = pos - start;  // Length of the word
            char* word = malloc(len + 1);
            strncpy(word, source + start, len);
            word[len] = '\0';
            
            // Check if it's a keyword (like "print")
            if (strcmp(word, "print") == 0) {
                tokens[token_pos++] = (Token){TOKEN_PRINT, word, line};
            } else {
                // Otherwise it's a variable name
                tokens[token_pos++] = (Token){TOKEN_IDENTIFIER, word, line};
            }
            continue;
        }

        // Handle numbers
        if (isdigit(source[pos])) {
            int start = pos;
            while (isdigit(source[pos])) pos++;
            int len = pos - start;
            char* num = malloc(len + 1);
            strncpy(num, source + start, len);
            num[len] = '\0';
            tokens[token_pos++] = (Token){TOKEN_NUMBER, num, line};
            continue;
        }
        
        // Handle single-character tokens: + = ( )
        switch (source[pos]) {
            case '+': tokens[token_pos++] = (Token){TOKEN_PLUS, NULL, line}; pos++; break;
            case '=': tokens[token_pos++] = (Token){TOKEN_EQUALS, NULL, line}; pos++; break;
            case '(': tokens[token_pos++] = (Token){TOKEN_LPAREN, NULL, line}; pos++; break;
            case ')': tokens[token_pos++] = (Token){TOKEN_RPAREN, NULL, line}; pos++; break;
            default: 
                pos++; // Skip any unknown characters
        }
    }

    // Add end-of-file marker
    tokens[token_pos++] = (Token){TOKEN_EOF, NULL, line};
    *count = token_pos;  // Return how many tokens we found
    return tokens;
}

// Parser state - keeps track of where we are in the token stream
typedef struct {
    Token* tokens;   // Array of tokens from lexer
    int pos;         // Current position
    int count;       // Total number of tokens
} Parser;

// Forward declaration (parse_expression calls parse_primary, which might call parse_expression)
static ASTNode* parse_expression(Parser* p);

// Parse the smallest piece: a string, identifier, or function call
static ASTNode* parse_primary(Parser* p) {
    Token* t = &p->tokens[p->pos];

    // Handle string literals: "Hello"
    if (t->type == TOKEN_STRING) {
        p->pos++;  // Consume the token
        ASTNode* node = malloc(sizeof(ASTNode));
        node->type = NODE_STRING;
        node->data.string.value = strdup(t->value);  // Copy the string
        return node;
    }
    
    // Handle numbers (simplified - store as string)
    if (t->type == TOKEN_NUMBER) {
        p->pos++;
        ASTNode* node = malloc(sizeof(ASTNode));
        node->type = NODE_STRING;  // Treat numbers as strings for simplicity
        node->data.string.value = strdup(t->value);
        return node;
    }

    // Handle identifiers (variable names) and function calls
    if (t->type == TOKEN_IDENTIFIER) {
        char* name = strdup(t->value);  // Save the name
        p->pos++;  // Consume the identifier
        
        // Check if it's a function call: name ( ... )
        if (p->pos < p->count && p->tokens[p->pos].type == TOKEN_LPAREN) {
            p->pos++; // Skip '('
            
            // Parse arguments (simplified - just expressions)
            ASTNode* args[10];  // Fixed size for simplicity
            int arg_count = 0;
            
            while (p->pos < p->count && p->tokens[p->pos].type != TOKEN_RPAREN) {
                args[arg_count++] = parse_expression(p);
                // Skip comma if present (simplified - just ignore)
            }
            
            if (p->tokens[p->pos].type == TOKEN_RPAREN) p->pos++; // Skip ')'
            
            // Create function call node
            ASTNode* node = malloc(sizeof(ASTNode));
            node->type = NODE_FUNC_CALL;
            node->data.call.name = name;
            node->data.call.args = malloc(sizeof(ASTNode*) * arg_count);
            memcpy(node->data.call.args, args, sizeof(ASTNode*) * arg_count);
            node->data.call.arg_count = arg_count;
            return node;
        }
        
        // Regular identifier (variable)
        ASTNode* node = malloc(sizeof(ASTNode));
        node->type = NODE_IDENTIFIER;
        node->data.identifier.name = name;
        return node;
    }
    
    return NULL;  // Unknown token
}

// Parse expressions with operators (like a + b)
static ASTNode* parse_expression(Parser* p) {
    // Start with a primary expression
    ASTNode* left = parse_primary(p);
    
    // Keep looking for operators
    while (p->pos < p->count) {
        Token* t = &p->tokens[p->pos];
        
        // Handle + operator (string concatenation)
        if (t->type == TOKEN_PLUS) {
            p->pos++;  // Consume the '+'
            ASTNode* right = parse_primary(p);  // Parse the right side
            
            // Create a binary operation node
            ASTNode* node = malloc(sizeof(ASTNode));
            node->type = NODE_BINOP;
            node->data.binop.left = left;
            node->data.binop.right = right;
            node->data.binop.op = '+';
            left = node;  // This becomes the new left for more operators
        } else {
            break;  // No more operators
        }
    }
    
    return left;
}

// Parse a single statement (print, assignment, or expression)
static ASTNode* parse_statement(Parser* p) {
    Token* t = &p->tokens[p->pos];
    
    // Handle print statements: print expression
    if (t->type == TOKEN_PRINT) {
        p->pos++;  // Consume 'print'
        ASTNode* expr = parse_expression(p);
        
        ASTNode* node = malloc(sizeof(ASTNode));
        node->type = NODE_PRINT;
        node->data.print.expr = expr;
        return node;
    }
    
    // Handle assignments: identifier = expression
    if (t->type == TOKEN_IDENTIFIER) {
        char* name = strdup(t->value);
        p->pos++;  // Consume identifier
        
        // Check for equals sign
        if (p->pos < p->count && p->tokens[p->pos].type == TOKEN_EQUALS) {
            p->pos++; // Skip '='
            ASTNode* value = parse_expression(p);
            
            ASTNode* node = malloc(sizeof(ASTNode));
            node->type = NODE_ASSIGN;
            node->data.assign.name = name;
            node->data.assign.value = value;
            return node;
        }
    }
    
    // Default: just an expression
    return parse_expression(p);
}

// Main parse function - entry point
ASTNode* parse(Token* tokens, int count) {
    Parser p = {tokens, 0, count};
    return parse_statement(&p);  // Parse one statement (simplified)
}

// Store a variable value
static void set_variable(Interpreter* interp, const char* name, const char* value) {
    // Look for existing variable with same name
    for (int i = 0; i < interp->var_count; i++) {
        if (strcmp(interp->var_names[i], name) == 0) {
            // Found it - update the value
            free(interp->var_values[i]);  // Free old value
            interp->var_values[i] = strdup(value);  // Store new value
            return;
        }
    }
    
    // Not found - add new variable
    // Grow arrays if needed
    if (interp->var_count >= interp->var_capacity) {
        interp->var_capacity = interp->var_capacity == 0 ? 10 : interp->var_capacity * 2;
        interp->var_names = realloc(interp->var_names, interp->var_capacity * sizeof(char*));
        interp->var_values = realloc(interp->var_values, interp->var_capacity * sizeof(char*));
    }
    
    // Store new variable
    interp->var_names[interp->var_count] = strdup(name);
    interp->var_values[interp->var_count] = strdup(value);
    interp->var_count++;
}

// Retrieve a variable value
static char* get_variable(Interpreter* interp, const char* name) {
    for (int i = 0; i < interp->var_count; i++) {
        if (strcmp(interp->var_names[i], name) == 0) {
            return interp->var_values[i];  // Return existing value
        }
    }
    return "";  // Not found - return empty string
}

// Evaluate a node and return its value as a string
static char* evaluate(Interpreter* interp, ASTNode* node) {
    if (!node) return strdup("");
    
    switch (node->type) {
        case NODE_STRING:
            // Just return the string value
            return strdup(node->data.string.value);
            
        case NODE_IDENTIFIER:
            // Look up the variable value
            return strdup(get_variable(interp, node->data.identifier.name));

        case NODE_FUNC_CALL: {
            // Handle built-in functions
            if (strcmp(node->data.call.name, "get_cpu_info") == 0) {
                return strdup(get_cpu_info());  // Call hardware function
            }
            if (strcmp(node->data.call.name, "get_ram_total") == 0) {
                char buf[64];
                sprintf(buf, "%llu", get_ram_total());  // Convert number to string
                return strdup(buf);
            }
            if (strcmp(node->data.call.name, "get_ram_used") == 0) {
                char buf[64];
                sprintf(buf, "%llu", get_ram_used());
                return strdup(buf);
            }
            if (strcmp(node->data.call.name, "get_os_name") == 0) {
                return strdup(get_os_name());
            }
            return strdup("");  // Unknown function
        }

        case NODE_BINOP: {
            // Evaluate left and right sides
            char* left = evaluate(interp, node->data.binop.left);
            char* right = evaluate(interp, node->data.binop.right);
            
            if (node->data.binop.op == '+') {
                // String concatenation
                char* result = malloc(strlen(left) + strlen(right) + 1);
                strcpy(result, left);
                strcat(result, right);
                free(left);
                free(right);
                return result;
            }
            free(left);
            free(right);
            return strdup("");
        }

        case NODE_PRINT: {
            // Evaluate what to print
            char* result = evaluate(interp, node->data.print.expr);
            printf("%s\n", result);  // Actually print it
            free(result);
            return strdup("");  // Print returns nothing
        }
            
        case NODE_ASSIGN: {
            // Evaluate the value
            char* value = evaluate(interp, node->data.assign.value);
            // Store it in the variable
            set_variable(interp, node->data.assign.name, value);
            free(value);
            return strdup("");  // Assignment returns nothing
        }
    }
    
    return strdup("");
}

// Main interpreter entry point
char* interpret(Interpreter* interp, ASTNode* node) {
    return evaluate(interp, node);
}

// Free an AST node and all its children
void free_ast(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PRINT:
            free_ast(node->data.print.expr);
            break;
        case NODE_ASSIGN:
            free(node->data.assign.name);
            free_ast(node->data.assign.value);
            break;
        case NODE_STRING:
            free(node->data.string.value);
            break;
        case NODE_IDENTIFIER:
            free(node->data.identifier.name);
            break;
        case NODE_FUNC_CALL:
            free(node->data.call.name);
            for (int i = 0; i < node->data.call.arg_count; i++) {
                free_ast(node->data.call.args[i]);
            }
            free(node->data.call.args);
            break;
        case NODE_BINOP:
            free_ast(node->data.binop.left);
            free_ast(node->data.binop.right);
            break;
    }
    free(node);
}

// Free tokens array
void free_tokens(Token* tokens, int count) {
    for (int i = 0; i < count; i++) {
        free(tokens[i].value);  // Free each token's string value
    }
    free(tokens);  // Free the array itself
}







// ==================== WINDOWS VERSION ====================
#ifdef _WIN32
#include <windows.h>

const char* get_cpu_info(void) {
    static char buffer[256];  // Static so we can return pointer
    HKEY hKey;
    DWORD type, size = sizeof(buffer);
    
    // Read CPU name from Windows Registry
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, "ProcessorNameString", NULL, 
                       &type, (LPBYTE)buffer, &size);
        RegCloseKey(hKey);
    } else {
        strcpy(buffer, "Unknown CPU");
    }
    return buffer;
}

unsigned long long get_ram_total(void) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return memInfo.ullTotalPhys;  // Total physical RAM in bytes
}

unsigned long long get_ram_used(void) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return memInfo.ullTotalPhys - memInfo.ullAvailPhys;  // Used = Total - Available
}

const char* get_os_name(void) {
    return "Windows";
}

// ==================== MACOS VERSION ====================
#elif defined(__APPLE__) || defined(__MACH__)
#include <sys/sysctl.h>
#include <mach/mach.h>

const char* get_cpu_info(void) {
    static char buffer[256];
    size_t len = sizeof(buffer);
    // Use sysctl to get CPU brand string
    sysctlbyname("machdep.cpu.brand_string", buffer, &len, NULL, 0);
    return buffer;
}

unsigned long long get_ram_total(void) {
    uint64_t mem;
    size_t len = sizeof(mem);
    sysctlbyname("hw.memsize", &mem, &len, NULL, 0);  // Get total RAM
    return mem;
}

unsigned long long get_ram_used(void) {
    // Get VM statistics
    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics_data_t vmstat;
    host_statistics(mach_host_self(), HOST_VM_INFO, 
                   (host_info_t)&vmstat, &count);
    
    // Get total RAM
    uint64_t total;
    size_t len = sizeof(total);
    sysctlbyname("hw.memsize", &total, &len, NULL, 0);
    
    // Calculate used: active + wired pages * page size
    int page_size = getpagesize();
    uint64_t used = (vmstat.active_count + vmstat.wire_count) * page_size;
    return used;
}

const char* get_os_name(void) {
    return "macOS";
}


// ==================== LINUX VERSION ====================
#elif defined(__linux__) || defined(__unix__)
#include <unistd.h>
#include <sys/sysinfo.h>

const char* get_cpu_info(void) {
    static char buffer[256];
    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if (!cpuinfo) return "Unknown CPU";
    
    char line[256];
    while (fgets(line, sizeof(line), cpuinfo)) {
        if (strncmp(line, "model name", 10) == 0) {
            char* colon = strchr(line, ':');
            if (colon) {
                strcpy(buffer, colon + 2);  // Copy after colon
                // Remove newline
                char* nl = strchr(buffer, '\n');
                if (nl) *nl = '\0';
                break;
            }
        }
    }
    fclose(cpuinfo);
    return buffer;
}

unsigned long long get_ram_total(void) {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return info.totalram * info.mem_unit;  // Total RAM in bytes
    }
    return 0;
}

unsigned long long get_ram_used(void) {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return (info.totalram - info.freeram) * info.mem_unit;  // Used RAM
    }
    return 0;
}

const char* get_os_name(void) {
    return "Linux";
}

// ==================== FALLBACK ====================
#else
const char* get_cpu_info(void) { return "Unknown CPU"; }
unsigned long long get_ram_total(void) { return 0; }
unsigned long long get_ram_used(void) { return 0; }
const char* get_os_name(void) { return "Unknown OS"; }
#endif