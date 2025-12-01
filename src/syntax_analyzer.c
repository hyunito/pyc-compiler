#include <stdio.h>
#include <string.h>

#define MAX_TOKENS 5000
#define MAX_IDENTIFIERS 1000
#define LOGGING 0             // 1 to enable, 0 to disable
#define DEBUG 0               // 1 to enable, 0 to disable

/* Default logging/debug flags (can be overridden at compile time) */
#ifndef LOGGING
#define LOGGING 0
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

typedef struct {
    char type[50];
    char value[100];
} Token;

typedef struct {
    char name[100];
    int isDeclared;
} Identifier;

Token tokens[MAX_TOKENS];
int tokenCount = 0;
int current = 0;
int errorCount = 0;

// Symbol table for tracking declared identifiers
Identifier symbolTable[MAX_IDENTIFIERS];
int symbolCount = 0;

static Token EOF_TOKEN = { "EOF", "" };

int isAtEnd() {
    return current >= tokenCount;
}

Token peekAt(int offset);

Token peek() {
    if (LOGGING) {
        printf("[LOG] peek(): Returning %s at position %d\n", 
               isAtEnd() ? "EOF" : tokens[current].type, current);
    }

    if (isAtEnd()) return EOF_TOKEN;
    return tokens[current];
}

Token peekAt(int offset) {
    int index = current + offset;
    if (index >= tokenCount) return EOF_TOKEN;
    return tokens[index];
}

Token peekNext() { return peekAt(1); }
Token peekNextNext() { return peekAt(2); }

void logger(const char* message) {
    if (LOGGING) {
        printf("[LOG] %s\n", message);
    }
}

void debug_state(const char* action) {
    if (DEBUG) {
        printf("[DEBUG] %s | Current: %s('%s') | Pos: %d/%d | Errors: %d\n", 
               action, peek().type, peek().value, current, tokenCount, errorCount);
    }
}

void advance() {
    if (DEBUG) {
        if (!isAtEnd())
            printf("[DEBUG] advance(): Moving from %s('%s') ", tokens[current].type, tokens[current].value);
        else
            printf("[DEBUG] advance(): At EOF\n");
    }
    if (!isAtEnd()) current++;
    if (DEBUG) {
        if (!isAtEnd())
            printf("to %s('%s')\n", tokens[current].type, tokens[current].value);
        else
            printf("to EOF\n");
    }
}

void recover() {
    if (LOGGING) {
        printf("[LOG] recover(): Starting error recovery at %s('%s')\n", 
               peek().type, peek().value);
    }

    if (isAtEnd()) return;
    advance();

    while (!isAtEnd()) {
        if (strcmp(tokens[current].type, "SEMICOLON") == 0) {
            advance();
            return;
        }
        if (strcmp(tokens[current].type, "RIGHT_BRACE") == 0) {
            return;
        }
        if (strcmp(tokens[current].type, "RESERVED_WORD") == 0 ||
            strcmp(tokens[current].type, "IDENTIFIER") == 0) {
            return;
        }
        if (strcmp(tokens[current].type, "KEYWORD") == 0) {
            if (strcmp(tokens[current].value, "if") == 0 ||
                strcmp(tokens[current].value, "for") == 0 ||
                strcmp(tokens[current].value, "while") == 0 ||
                strcmp(tokens[current].value, "return") == 0) {
                return;
            }
        }
        advance();
    }
}

int match(const char* expected) {
    if (LOGGING) {
        printf("[LOG] match(): Looking for '%s', found '%s'\n", 
               expected, peek().type);
    }

    if (strcmp(peek().type, expected) == 0) {
        advance();
        return 1;
    }

    printf("Syntax Error: Expected %s but got %s (%s)\n",
           expected, peek().type, peek().value);

    errorCount++;
    recover();
    return 0;
}

// Add identifier to symbol table
void declareIdentifier(const char* name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return; // Already declared
        }
    }
    if (symbolCount < MAX_IDENTIFIERS) {
        strncpy(symbolTable[symbolCount].name, name, sizeof(symbolTable[symbolCount].name)-1);
        symbolTable[symbolCount].name[sizeof(symbolTable[symbolCount].name)-1] = '\0';
        symbolTable[symbolCount].isDeclared = 1;
        symbolCount++;
    }
}

// Check if identifier is declared
int isIdentifierDeclared(const char* name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

int isKeyword(const char* k) {
    Token t = peek();
    return strcmp(t.type, "KEYWORD") == 0 && strcmp(t.value, k) == 0;
}

int isKeywordAt(int offset, const char* k) {
    Token t = peekAt(offset);
    return strcmp(t.type, "KEYWORD") == 0 && strcmp(t.value, k) == 0;
}
int isTypeValue(const char* val) {
    return strcmp(val, "int") == 0 ||
           strcmp(val, "float") == 0 ||
           strcmp(val, "bool") == 0 ||
           strcmp(val, "char") == 0 ||
           strcmp(val, "str") == 0 ||
           strcmp(val, "void") == 0;
}

int findMain() {
    for (int i = 0; i < tokenCount - 3; i++) {
        if (strcmp(tokens[i].type, "KEYWORD") == 0 &&
            strcmp(tokens[i].value, "main") == 0 &&
            strcmp(tokens[i+1].type, "LEFT_PAREN") == 0 &&
            strcmp(tokens[i+2].type, "RIGHT_PAREN") == 0 &&
            strcmp(tokens[i+3].type, "LEFT_BRACE") == 0) {
            return i;
        }
    }
    return -1;
}

void parseProgram();
void parseBlock();

void parseStatement();
void parseDeclaration();
void parseAssignment();
void parseIf();
void parseWhile();
void parseFor();
void parseOutput();
void parseInputStatement();
void parseFunctionDef();
void parseFunctionCall();
void parseArrayAssignment();
void skipComment();

void parseExpression();
void parseLogicalOr();
void parseLogicalAnd();
void parseLogicalNot();
void parseRelational();
void parseArithmetic();
void parseTerm();
void parseFactor();

void parseProgram() {
    int start = findMain();
    if (start == -1) {
        printf("Syntax Error: No main() function found.\n");
        return;
    }

    current = start;

    if (!match("KEYWORD")) return; 
    match("LEFT_PAREN");
    match("RIGHT_PAREN");

    parseBlock();
}

void parseBlock() {
    match("LEFT_BRACE");

    while (!isAtEnd() && strcmp(peek().type, "RIGHT_BRACE") != 0) {
        parseStatement();
    }

    match("RIGHT_BRACE");

    if (isKeyword("end")) match("KEYWORD");
}

void parseStatement() {
    if (strcmp(peek().type, "COMMENT") == 0 ||
        strcmp(peek().type, "COMMENT_START") == 0) {
        skipComment();
        return;
    }

    if (isKeyword("if")) { parseIf(); return; }

    if (isKeyword("while")) { parseWhile(); return; }

    if (isKeyword("for")) { parseFor(); return; }

    if (isKeyword("break")) { match("KEYWORD"); match("SEMICOLON"); return; }
    if (isKeyword("continue")) { match("KEYWORD"); match("SEMICOLON"); return; }

    if (isKeyword("return")) {
        match("KEYWORD");
        parseExpression();
        match("SEMICOLON");
        return;
    }

    if (isKeyword("output")) { parseOutput(); return; }

    if (strcmp(peek().type, "IDENTIFIER") == 0 &&
        strcmp(peekNext().type, "ASSIGN_OP") == 0 &&
        isKeywordAt(2, "input")) {
        parseInputStatement();
        return;
    }

    if (strcmp(peek().type, "RESERVED_WORD") == 0 &&
        isTypeValue(peek().value) &&
        strcmp(peekNext().type, "IDENTIFIER") == 0 &&
        strcmp(peekAt(2).type, "LEFT_PAREN") == 0) {
        parseFunctionDef();
        return;
    }

    if (isKeyword("begin")) { match("KEYWORD"); parseBlock(); if (isKeyword("end")) match("KEYWORD"); return; }
    if (strcmp(peek().type, "LEFT_BRACE") == 0) { parseBlock(); return; }

    if (strcmp(peek().type, "RESERVED_WORD") == 0 && isTypeValue(peek().value)) {
        parseDeclaration();
        return;
    }

    if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "LEFT_PAREN") == 0) {
        parseFunctionCall();
        match("SEMICOLON");
        return;
    }

    if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "LEFT_BRACKET") == 0) {
        parseArrayAssignment();
        match("SEMICOLON");
        return;
    }

    if (strcmp(peek().type, "INCREMENT") == 0 || strcmp(peek().type, "DECREMENT") == 0) {
        advance();
        match("IDENTIFIER");
        match("SEMICOLON");
        return;
    }

    if (strcmp(peek().type, "IDENTIFIER") == 0 &&
       (strcmp(peekNext().type, "INCREMENT") == 0 || strcmp(peekNext().type, "DECREMENT") == 0)) {
        match("IDENTIFIER");
        advance();
        match("SEMICOLON");
        return;
    }
    if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "ASSIGN_OP") == 0) {
        parseAssignment();
        return;
    }

    printf("Syntax Error: Unknown statement starting with %s (%s)\n", peek().type, peek().value);
    errorCount++;
    recover();
}

void parseDeclaration() {
    /* type id (= expr)? (, id (=expr)? )* ; */
    match("RESERVED_WORD");

    if (strcmp(peek().type, "IDENTIFIER") != 0) {
        printf("Syntax Error: Expected identifier in declaration.\n");
        errorCount++;
        recover();
        return;
    }

    while (1) {
        char idname[100];
        strncpy(idname, peek().value, sizeof(idname)-1);
        idname[sizeof(idname)-1] = '\0';
        match("IDENTIFIER");

        if (strcmp(peek().type, "ASSIGN_OP") == 0) {
            match("ASSIGN_OP");
            parseExpression();
        }

        declareIdentifier(idname);

        if (strcmp(peek().type, "COMMA") == 0) {
            match("COMMA");
            continue;
        } else break;
    }

    match("SEMICOLON");
}

void parseIf() {
    match("KEYWORD");
    match("LEFT_PAREN");
    parseExpression();
    match("RIGHT_PAREN");
    parseBlock();
    
    while (isKeyword("else")) {
        match("KEYWORD");
        if (isKeyword("if")) {
            match("KEYWORD");
            match("LEFT_PAREN");
            parseExpression();
            match("RIGHT_PAREN");
            parseBlock();
        } else {
            
            parseBlock();
            break;
        }
    }
}

void parseWhile() {
    match("KEYWORD");
    match("LEFT_PAREN");
    parseExpression();
    match("RIGHT_PAREN");
    if (isKeyword("do")) match("KEYWORD");
    parseBlock();
}

void parseFor() {
    match("KEYWORD");
    match("LEFT_PAREN");

    
    if (strcmp(peek().type, "SEMICOLON") == 0) {
        match("SEMICOLON");
    } else if (strcmp(peek().type, "RESERVED_WORD") == 0 && isTypeValue(peek().value)) {
        parseDeclaration();
    } else {
        
        parseAssignment();
        match("SEMICOLON");
    }

    if (strcmp(peek().type, "SEMICOLON") == 0) {
        match("SEMICOLON");
    } else {
        parseExpression();
        match("SEMICOLON");
    }
    if (strcmp(peek().type, "RIGHT_PAREN") == 0) {
        /* nothing */
    } else {
        if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "ASSIGN_OP") == 0) {
            parseAssignment();
        } else if (strcmp(peek().type, "IDENTIFIER") == 0 && (strcmp(peekNext().type, "INCREMENT") == 0 || strcmp(peekNext().type, "DECREMENT") == 0)) {
            match("IDENTIFIER");
            advance();
        } else if (strcmp(peek().type, "INCREMENT") == 0 || strcmp(peek().type, "DECREMENT") == 0) {
            advance();
            match("IDENTIFIER");
        } else {
            parseExpression();
        }
    }

    match("RIGHT_PAREN");

    if (isKeyword("do")) match("KEYWORD");
    parseBlock();
}

void parseAssignment() {
    if (strcmp(peek().type, "IDENTIFIER") != 0) {
        printf("Syntax Error: Assignment must start with identifier.\n");
        errorCount++;
        recover();
        return;
    }
    char namebuf[100];
    strncpy(namebuf, peek().value, sizeof(namebuf)-1);
    namebuf[sizeof(namebuf)-1] = '\0';

    match("IDENTIFIER");
    match("ASSIGN_OP");

    
    if (strcmp(peek().type, "KEYWORD") == 0 && strcmp(peek().value, "input") == 0) {
        match("KEYWORD");
        match("LEFT_PAREN");
        match("STRING");
        match("RIGHT_PAREN");
    } else {
        parseExpression();
    }

    match("SEMICOLON");

    if (!isIdentifierDeclared(namebuf)) {
        printf("Warning: Identifier '%s' assigned but not declared.\n", namebuf);
    }
}

void parseArrayAssignment() {
    
    match("IDENTIFIER");
    match("LEFT_BRACKET");

    if (strcmp(peek().type, "RIGHT_BRACKET") != 0) {
        parseExpression();
    }
    match("RIGHT_BRACKET");

    match("ASSIGN_OP");

    if (strcmp(peek().type, "LEFT_BRACE") == 0) {
        match("LEFT_BRACE");
        if (strcmp(peek().type, "RIGHT_BRACE") != 0) {
            parseExpression();
            while (strcmp(peek().type, "COMMA") == 0) {
                match("COMMA");
                parseExpression();
            }
        }
        match("RIGHT_BRACE");
    } else {
        parseExpression();
    }
}

void parseInputStatement() {
    if (strcmp(peek().type, "IDENTIFIER") != 0) {
        printf("Syntax Error: input assignment must start with identifier.\n");
        errorCount++;
        recover();
        return;
    }
    match("IDENTIFIER");
    match("ASSIGN_OP");
    match("KEYWORD");
    match("LEFT_PAREN");
    match("STRING");
    match("RIGHT_PAREN");
    match("SEMICOLON");
}

void parseOutput() {
    match("KEYWORD");
    match("LEFT_PAREN");

    if (strcmp(peek().type, "RIGHT_PAREN") != 0) {
        if (strcmp(peek().type, "STRING") == 0) {
            match("STRING");
        } else {
            parseExpression();
        }

        while (strcmp(peek().type, "COMMA") == 0) {
            match("COMMA");
            if (strcmp(peek().type, "STRING") == 0) {
                match("STRING");
            } else {
                parseExpression();
            }
        }
    }

    match("RIGHT_PAREN");
    match("SEMICOLON");
}

void parseFunctionCall() {
    /* Function call: id(args) */
    match("IDENTIFIER");
    match("LEFT_PAREN");
    if (strcmp(peek().type, "RIGHT_PAREN") != 0) {
        parseExpression();
        while (strcmp(peek().type, "COMMA") == 0) {
            match("COMMA");
            parseExpression();
        }
    }
    match("RIGHT_PAREN");
}

void parseFunctionDef() {
    match("RESERVED_WORD");
    if (strcmp(peek().type, "IDENTIFIER") != 0) {
        printf("Syntax Error: Expected function name.\n");
        errorCount++;
        recover();
        return;
    }
    
    char fname[100];
    strncpy(fname, peek().value, sizeof(fname)-1);
    fname[sizeof(fname)-1] = '\0';
    match("IDENTIFIER");

    match("LEFT_PAREN");
    
    if (!(strcmp(peek().type, "RIGHT_PAREN") == 0)) {
        while (1) {
            if (strcmp(peek().type, "RESERVED_WORD") != 0 || !isTypeValue(peek().value)) {
                printf("Syntax Error: Expected type in parameter list.\n");
                errorCount++;
                recover();
                return;
            }
            match("RESERVED_WORD");
            if (strcmp(peek().type, "IDENTIFIER") != 0) {
                printf("Syntax Error: Expected parameter name.\n");
                errorCount++;
                recover();
                return;
            }
            declareIdentifier(peek().value);
            match("IDENTIFIER");

            if (strcmp(peek().type, "COMMA") == 0) {
                match("COMMA");
                continue;
            } else break;
        }
    }
    match("RIGHT_PAREN");

    
    parseBlock();
}

void parseExpression() {
    parseLogicalOr();
}

void parseLogicalOr() {
    parseLogicalAnd();
    while (strcmp(peek().type, "LOGICAL") == 0 && strcmp(peek().value, "or") == 0) {
        match("LOGICAL");
        parseLogicalAnd();
    }
}

void parseLogicalAnd() {
    parseLogicalNot();
    while (strcmp(peek().type, "LOGICAL") == 0 && strcmp(peek().value, "and") == 0) {
        match("LOGICAL");
        parseLogicalNot();
    }
}

void parseLogicalNot() {
    if (strcmp(peek().type, "LOGICAL") == 0 && strcmp(peek().value, "not") == 0) {
        match("LOGICAL");
        parseLogicalNot();
        return;
    }
    parseRelational();
}

void parseRelational() {
    parseArithmetic();
    while (strcmp(peek().type, "REL_OP") == 0) {
        match("REL_OP");
        parseArithmetic();
    }
}

void parseArithmetic() {
    parseTerm();
    while (strcmp(peek().type, "ARITH_OP") == 0 &&
          (strcmp(peek().value, "+") == 0 || strcmp(peek().value, "-") == 0)) {
        match("ARITH_OP");
        parseTerm();
    }
}

void parseTerm() {
    parseFactor();
    while (strcmp(peek().type, "ARITH_OP") == 0 &&
          (strcmp(peek().value, "*") == 0 ||
           strcmp(peek().value, "/") == 0 ||
           strcmp(peek().value, "%") == 0 ||
           strcmp(peek().value, "//") == 0)) {
        match("ARITH_OP");
        parseFactor();
    }
}

void parseFactor() {
    if (strcmp(peek().type, "ARITH_OP") == 0 &&
        (strcmp(peek().value, "+") == 0 || strcmp(peek().value, "-") == 0)) {
        match("ARITH_OP");
        parseFactor();
        return;
    }
    if (strcmp(peek().type, "INCREMENT") == 0 || strcmp(peek().type, "DECREMENT") == 0) {
        match(peek().type);
        match("IDENTIFIER");
        return;
    }
    if (strcmp(peek().type, "NUMBER") == 0 ||
        strcmp(peek().type, "STRING") == 0 ||
        strcmp(peek().type, "CHAR") == 0) {
        advance();
    } else if (strcmp(peek().type, "IDENTIFIER") == 0) {
        
        if (strcmp(peekNext().type, "LEFT_PAREN") == 0) {
            parseFunctionCall();
        } else {
            if (!isIdentifierDeclared(peek().value)) {
                printf("Semantic Error: Identifier '%s' used before declaration\n", peek().value);
                errorCount++;
            }
            match("IDENTIFIER");
        }
    } else if (strcmp(peek().type, "LEFT_PAREN") == 0) {
        match("LEFT_PAREN");
        parseExpression();
        if (strcmp(peek().type, "RIGHT_PAREN") != 0) {
            printf("Syntax Error: Missing closing parenthesis\n");
            errorCount++;
            recover();
            return;
        }
        match("RIGHT_PAREN");
    } else if (strcmp(peek().type, "RESERVED_WORD") == 0 &&
               (strcmp(peek().value, "True") == 0 || strcmp(peek().value, "False") == 0)) {
        
        advance();
    } else {
        printf("Syntax Error: Missing operand - Expected NUMBER, IDENTIFIER, STRING, or '(' but got %s (%s)\n",
               peek().type, peek().value);
        errorCount++;
        recover();
        return;
    }
    if (strcmp(peek().type, "ARITH_OP") == 0 && strcmp(peek().value, "**") == 0) {
        match("ARITH_OP");
        parseFactor();
    }
}

void skipComment() {
    if (strcmp(peek().type, "COMMENT") == 0 || strcmp(peek().type, "COM_STR") == 0) {
        advance();
        return;
    }

    if (strcmp(peek().type, "COMMENT_START") == 0) {
        advance();
        while (!isAtEnd() && strcmp(peek().type, "COMMENT_END") != 0) {
            advance();
        }
        if (!isAtEnd()) advance();
        return;
    }

    advance();
}

int main() {
    FILE *fp = fopen("Symbol_Table.txt", "r");
    if (!fp) {
        printf("Error: Cannot open Symbol_Table.txt\n");
        return 1;
    }

    char line[256];

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strlen(line) < 3) continue;
        line[strcspn(line, "\n")] = '\0';

        char *open = strchr(line, '(');
        char *close = strrchr(line, ')');

        if (!open || !close || close < open) {
            printf("Warning: Skipping malformed line: %s\n", line);
            continue;
        }

        int typeLength = open - line;
        if (typeLength >= sizeof(tokens[tokenCount].type)) {
            typeLength = sizeof(tokens[tokenCount].type) - 1;
        }
        strncpy(tokens[tokenCount].type, line, typeLength);
        tokens[tokenCount].type[typeLength] = '\0';

        int valueLength = close - open - 1;
        if (valueLength >= sizeof(tokens[tokenCount].value)) {
            valueLength = sizeof(tokens[tokenCount].value) - 1;
        }
        if (valueLength > 0) {
            strncpy(tokens[tokenCount].value, open + 1, valueLength);
            tokens[tokenCount].value[valueLength] = '\0';
        } else {
            tokens[tokenCount].value[0] = '\0';
        }

        tokenCount++;
        if (tokenCount >= MAX_TOKENS) break;
    }

    fclose(fp);

    parseProgram();

    if (LOGGING || errorCount > 0) {
        printf("Parsing finished. Total errors: %d\n", errorCount);
    }

    return 0;
}
