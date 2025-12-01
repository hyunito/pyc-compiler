#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS 999999
#define MAX_IDENTIFIERS 1000
#define TRACK 0

typedef struct {
    char type[50];
    char value[100];
} Token;

typedef struct {
    char name[100];
    char type[20];
    int isDeclared;
} Identifier;

Token tokens[MAX_TOKENS];
int tokenCount = 0;
int current = 0;
int errorCount = 0;
int lines = 1;

Identifier symbolTable[MAX_IDENTIFIERS];
int symbolCount = 0;

static Token EOF_TOKEN = { "EOF", "" };

int isAtEnd() {
    return current >= tokenCount;
}

Token peekAt(int offset);

Token peek() {
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

void logTransition(const char* from, const char* input, const char* to) {
    if (TRACK) printf("%s --%s--> %s\n", from, input, to);
}

void advance() {
    if(isAtEnd()) return;

    while (!isAtEnd() && strcmp(tokens[current].type, "NEW_LINE") == 0) {
        lines++;
        current++;
    }

    if (!isAtEnd()) current++;

    while (!isAtEnd() && strcmp(tokens[current].type, "NEW_LINE") == 0) {
        lines++;
        current++;
    }
}

void recover() {
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
    if (strcmp(peek().type, expected) == 0) {
        advance();
        return 1;
    }
    printf("Syntax Error at Line %d: Expected %s but got %s (%s)\n", lines,
           expected, peek().type, peek().value);
    errorCount++;
    recover();
    return 0;
}

void declareIdentifier(const char* name, const char* type) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return;
        }
    }
    if (symbolCount < MAX_IDENTIFIERS) {
        strncpy(symbolTable[symbolCount].name, name, sizeof(symbolTable[symbolCount].name)-1);
        symbolTable[symbolCount].name[sizeof(symbolTable[symbolCount].name)-1] = '\0';
        
        strncpy(symbolTable[symbolCount].type, type, sizeof(symbolTable[symbolCount].type)-1);
        symbolTable[symbolCount].type[sizeof(symbolTable[symbolCount].type)-1] = '\0';
        
        symbolTable[symbolCount].isDeclared = 1;
        symbolCount++;
    }
}

int isIdentifierDeclared(const char* name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

const char* getIdentifierType(const char* name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return symbolTable[i].type;
        }
    }
    return "unknown";
}

void validateStringInterpolation(const char* content) {
    int len = strlen(content);
    char buffer[100];
    int bufIdx = 0;
    int insideBraces = 0;

    for(int i = 0; i < len; i++) {
        if (content[i] == '{') {
            insideBraces = 1;
            bufIdx = 0;
            continue;
        }
        
        if (content[i] == '}') {
            if (insideBraces) {
                buffer[bufIdx] = '\0';
                insideBraces = 0;
                
                if (bufIdx > 0) {
                    if (!isIdentifierDeclared(buffer)) {
                        printf("Syntax Error at Line %d: Identifier '%s' inside string interpolation is not declared.\n", lines, buffer);
                        errorCount++;
                    }
                }
            }
            continue;
        }

        if (insideBraces) {
            if (bufIdx < 99) {
                buffer[bufIdx++] = content[i];
            }
        }
    }
}

int isKeyword(const char* k) {
    Token t = peek();
    return strcmp(t.type, "KEYWORD") == 0 && strcmp(t.value, k) == 0;
}

int isNoise(const char* w) {
    return strcmp(peek().type, "NOISE_WORD") == 0 &&
           strcmp(peek().value, w) == 0;
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
            (strcmp(tokens[i+3].type, "NOISE_WORD") == 0 ||
            strcmp(tokens[i+3].type, "LEFT_BRACE") == 0)) {
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
int expectCondition();
void parseIf();
void parseWhile();
void parseFor();
void parseOutput();
void parseInputStatement();
void parseFunctionDef();
void parseFunctionCall();
void parseArrayAssignment();
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
    if (isNoise("begin")) {
        match("NOISE_WORD");
    }

    match("LEFT_BRACE");

    while (!isAtEnd() && strcmp(peek().type, "RIGHT_BRACE") != 0) {
        parseStatement();
    }

    match("RIGHT_BRACE");

    if (isNoise("end")) match("NOISE_WORD");
}

void parseStatement() {
    if (isKeyword("if")) {
        parseIf();
        return;
    }
    if (isKeyword("while")) {
        parseWhile();
        return;
    }

    if (isKeyword("for")) {
        parseFor();
        return;
    }

    if (isKeyword("break")) {
        match("KEYWORD");
        match("SEMICOLON");
        return;
    }
    if (isKeyword("continue")) {
        match("KEYWORD");
        match("SEMICOLON");
        return;
    }

    if (isKeyword("return")) {
        match("KEYWORD");
        parseExpression();
        match("SEMICOLON");
        return;
    }

    if (isKeyword("output")) {
        parseOutput();
        return;
    }

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

    if (isNoise("begin")) {
        match("NOISE_WORD");
        parseBlock();
        return;
    }
    if (strcmp(peek().type, "LEFT_BRACE") == 0) {
        parseBlock();
        return;
    }

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
    if (strcmp(peek().type, "NEW_LINE") == 0) {
        advance();
    }

    printf("Syntax Error at Line %d: Unknown statement starting with %s (%s)\n",lines, peek().type, peek().value);
    errorCount++;
    recover();
}

void parseDeclaration() {
    char declaredType[20];
    strncpy(declaredType, peek().value, sizeof(declaredType)-1);
    declaredType[sizeof(declaredType)-1] = '\0';
    
    match("RESERVED_WORD");

    if (strcmp(peek().type, "IDENTIFIER") != 0) {
        printf("Syntax Error at Line %d: Expected identifier in declaration.\n", lines);
        errorCount++;
        recover();
        return;
    }

    while (1) {
        char idname[100];
        strncpy(idname, peek().value, sizeof(idname)-1);
        idname[sizeof(idname)-1] = '\0';
        match("IDENTIFIER");

        declareIdentifier(idname, declaredType);

        if (strcmp(peek().type, "ASSIGN_OP") == 0) {
            match("ASSIGN_OP");
            
            if (strcmp(peek().type, "RESERVED_WORD") == 0 && isTypeValue(peek().value) && 
                strcmp(peekNext().type, "LEFT_PAREN") == 0 && isKeywordAt(2, "input")) {
                
                char castType[20];
                strncpy(castType, peek().value, sizeof(castType)-1);
                castType[sizeof(castType)-1] = '\0';
                
                if (strcmp(declaredType, castType) != 0) {
                    printf("Syntax Error at Line %d: Type mismatch. Cannot assign %s(input) to variable of type %s.\n", lines, castType, declaredType);
                    errorCount++;
                }
                
                match("RESERVED_WORD");
                match("LEFT_PAREN");
                match("KEYWORD");
                match("LEFT_PAREN");
                match("STRING");
                match("RIGHT_PAREN");
                match("RIGHT_PAREN");
                
            } 

            else if (isKeyword("input")) {
                if (strcmp(declaredType, "str") != 0 && strcmp(declaredType, "char") != 0) {
                     printf("Syntax Error at Line %d: Type mismatch. 'input()' returns char/str, but variable is type %s.\n", lines, declaredType);
                     errorCount++;
                }
                
                match("KEYWORD");
                match("LEFT_PAREN");
                match("STRING");
                match("RIGHT_PAREN");
            } 
            else {
                parseExpression();
            }
        }

        if (strcmp(peek().type, "COMMA") == 0) {
            match("COMMA");
            continue;
        } else break;
    }

    match("SEMICOLON");
}

int expectCondition() {
    if (strcmp(peek().type, "LEFT_PAREN") != 0) {
        printf("Syntax Error at Line %d: Expected '(' to start condition but got %s (%s)\n", lines,
               peek().type, peek().value);
        errorCount++;
        recover();
        return 0;
    }
    match("LEFT_PAREN");

    if (isAtEnd() || strcmp(peek().type, "RIGHT_PAREN") == 0) {
        printf("Syntax Error at Line %d: Expected condition expression inside parentheses.\n", lines);
        errorCount++;
        recover();
        return 0;
    }
    parseExpression();

    if (strcmp(peek().type, "RIGHT_PAREN") != 0) {
        printf("Syntax Error at Line %d: Expected ')' to close condition but got %s (%s)\n", lines,
               peek().type, peek().value);
        errorCount++;
        recover();
        return 0;
    }
    match("RIGHT_PAREN");
    return 1;
}

void parseIf() {
    if (strcmp(peek().type, "KEYWORD") != 0 || strcmp(peek().value, "if") != 0) {
        printf("Internal Error: parseIf called but current token is %s (%s)\n", peek().type, peek().value);
        errorCount++;
        recover();
        return;
    }
    match("KEYWORD");
    if (!expectCondition()) {
        if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
            parseBlock();
        } else {
            parseStatement();
        }
        return;
    }

    if (isNoise("then")) match("NOISE_WORD");
    if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
        parseBlock();
    } else {
        parseStatement();
    }

    while (isKeyword("else")) {
        match("KEYWORD");

        if (isKeyword("if")) {
            match("KEYWORD");
            if (!expectCondition()) {
                if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
                    parseBlock();
                } else {
                    parseStatement();
                }
                break;
            }
            if (isNoise("then")) match("NOISE_WORD");

            if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
                parseBlock();
            } else {
                parseStatement();
            }
        } else {
            if (isNoise("then")) match("NOISE_WORD");
            if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
                parseBlock();
            } else {
                parseStatement();
            }
            break;
        }
    }
}

void parseWhile() {
    if (strcmp(peek().type, "KEYWORD") != 0 || strcmp(peek().value, "while") != 0) {
        printf("Internal Error: parseWhile called but current token is %s (%s)\n", peek().type, peek().value);
        errorCount++;
        recover();
        return;
    }
    match("KEYWORD");
    if (!expectCondition()) {
        if (isNoise("do")) match("NOISE_WORD");
        if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
            parseBlock();
        } else {
            parseStatement();
        }
        return;
    }
    if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin") || isNoise("do")) {
        if (isNoise("do")) match("NOISE_WORD");
        parseBlock();
    } else {
        parseStatement();
    }
}

void parseFor() {
    if (strcmp(peek().type, "KEYWORD") != 0 || strcmp(peek().value, "for") != 0) {
        printf("Internal Error: parseFor called but current token is %s (%s)\n", peek().type, peek().value);
        errorCount++;
        recover();
        return;
    }
    match("KEYWORD");

    if (strcmp(peek().type, "LEFT_PAREN") != 0) {
        printf("Syntax Error at Line %d: Expected '(' after for but got %s (%s)\n", lines, peek().type, peek().value);
        errorCount++;
        recover();
    } else {
        match("LEFT_PAREN");
    }

    int initConsumedSemicolon = 0;
    if (strcmp(peek().type, "SEMICOLON") == 0) {
        match("SEMICOLON");
        initConsumedSemicolon = 1;
    } else if (strcmp(peek().type, "RESERVED_WORD") == 0 && isTypeValue(peek().value)) {
        parseDeclaration();
        initConsumedSemicolon = 1;
    } else {
        if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "ASSIGN_OP") == 0) {
            parseAssignment();
            initConsumedSemicolon = 1;
        } else {
            parseExpression();
            if (strcmp(peek().type, "SEMICOLON") == 0) {
                match("SEMICOLON");
                initConsumedSemicolon = 1;
            } else {
                printf("Syntax Error at Line %d: Expected ';' after for-initialization but got %s (%s)\n", lines,
                       peek().type, peek().value);
                errorCount++;
                recover();
            }
        }
    }

    if (!initConsumedSemicolon) {
        if (strcmp(peek().type, "SEMICOLON") == 0) {
            match("SEMICOLON");
        } else {
            printf("Syntax Error at Line %d: Expected ';' after for-initialization but got %s (%s)\n", lines,
                   peek().type, peek().value);
            errorCount++;
            recover();
        }
    }

    if (strcmp(peek().type, "SEMICOLON") == 0) {
        match("SEMICOLON");
    } else {
        parseExpression();
        if (strcmp(peek().type, "SEMICOLON") == 0) {
            match("SEMICOLON");
        } else {
            printf("Syntax Error at Line %d: Expected ';' after for-condition but got %s (%s)\n", lines,
                   peek().type, peek().value);
            errorCount++;
            recover();
        }
    }


    if (strcmp(peek().type, "RIGHT_PAREN") == 0) {
    } else {
        if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "ASSIGN_OP") == 0) {
            char namebuf[100];
            strncpy(namebuf, peek().value, sizeof(namebuf)-1);
            namebuf[sizeof(namebuf)-1] = '\0';
            match("IDENTIFIER");
            match("ASSIGN_OP");
            parseExpression();
            if (!isIdentifierDeclared(namebuf)) {
                printf("Warning: Identifier '%s' assigned but not declared.\n", namebuf);
            }
        } else if (strcmp(peek().type, "IDENTIFIER") == 0 &&
                   (strcmp(peekNext().type, "INCREMENT") == 0 || strcmp(peekNext().type, "DECREMENT") == 0)) {
            match("IDENTIFIER");
            advance();
        } else if (strcmp(peek().type, "INCREMENT") == 0 || strcmp(peek().type, "DECREMENT") == 0) {
            advance();
            match("IDENTIFIER");
        } else {
            parseExpression();
        }
    }

    if (strcmp(peek().type, "RIGHT_PAREN") == 0) {
        match("RIGHT_PAREN");
    } else {
        printf("Syntax Error at Line %d: Expected ')' to close for loop header but got %s (%s)\n", lines,
               peek().type, peek().value);
        errorCount++;
        recover();
    }


    if (isNoise("do")) match("NOISE_WORD");

    if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
        parseBlock();
    } else {
        parseStatement();
    }
}

void parseAssignment() {
    if (strcmp(peek().type, "IDENTIFIER") != 0) {
        printf("Syntax Error at Line %d: Assignment must start with identifier.\n", lines);
        errorCount++;
        recover();
        return;
    }
    
    char namebuf[100];
    strncpy(namebuf, peek().value, sizeof(namebuf)-1);
    namebuf[sizeof(namebuf)-1] = '\0';
    
    const char* targetType = getIdentifierType(namebuf);
    
    match("IDENTIFIER");
    match("ASSIGN_OP");

    if (strcmp(peek().type, "RESERVED_WORD") == 0 && isTypeValue(peek().value) && 
        strcmp(peekNext().type, "LEFT_PAREN") == 0 && isKeywordAt(2, "input")) {
        
        char castType[20];
        strncpy(castType, peek().value, sizeof(castType)-1);
        castType[sizeof(castType)-1] = '\0';

        if (strcmp(targetType, "unknown") != 0 && strcmp(targetType, castType) != 0) {
             printf("Syntax Error at Line %d: Type mismatch. Cannot assign %s(input) to variable '%s' of type %s.\n", lines, castType, namebuf, targetType);
             errorCount++;
        }

        match("RESERVED_WORD");
        match("LEFT_PAREN");
        match("KEYWORD");
        match("LEFT_PAREN");
        match("STRING");
        match("RIGHT_PAREN");
        match("RIGHT_PAREN");
    }
    else if (strcmp(peek().type, "KEYWORD") == 0 && strcmp(peek().value, "input") == 0) {
        if (strcmp(targetType, "unknown") != 0 && strcmp(targetType, "str") != 0 && strcmp(targetType, "char") != 0) {
             printf("Syntax Error at Line %d: Type mismatch. 'input()' returns char/str, but variable '%s' is type %s.\n", lines, namebuf, targetType);
             errorCount++;
        }
        
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
    match("SEMICOLON");
}

void parseInputStatement() {
    if (strcmp(peek().type, "IDENTIFIER") != 0) {
        printf("Syntax Error at Line %d: input assignment must start with identifier.\n", lines);
        errorCount++;
        recover();
        return;
    }
    
    char namebuf[100];
    strncpy(namebuf, peek().value, sizeof(namebuf)-1);
    namebuf[sizeof(namebuf)-1] = '\0';
    
    if (!isIdentifierDeclared(namebuf)) {
        printf("Syntax Error at Line %d: Identifier '%s' is not declared.\n", lines, namebuf);
        errorCount++;
    }
    
    const char* targetType = getIdentifierType(namebuf);
    
    match("IDENTIFIER");
    match("ASSIGN_OP");
    match("KEYWORD");
    
    if (strcmp(targetType, "unknown") != 0 && strcmp(targetType, "str") != 0 && strcmp(targetType, "char") != 0) {
        printf("Syntax Error at Line %d: Type mismatch. 'input()' returns char/str, but variable '%s' is type %s.\n", lines, namebuf, targetType);
        errorCount++;
    }

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
            validateStringInterpolation(peek().value);
            match("STRING");
        } else {
            parseExpression();
        }

        while (strcmp(peek().type, "COMMA") == 0) {
            match("COMMA");
            if (strcmp(peek().type, "STRING") == 0) {
                validateStringInterpolation(peek().value);
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
        printf("Syntax Error at Line %d: Expected function name.\n", lines);
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
                printf("Syntax Error at Line %d: Expected type in parameter list.\n", lines);
                errorCount++;
                recover();
                return;
            }
            char paramType[20];
            strncpy(paramType, peek().value, sizeof(paramType)-1);
            paramType[sizeof(paramType)-1] = '\0';
            
            match("RESERVED_WORD");
            
            if (strcmp(peek().type, "IDENTIFIER") != 0) {
                printf("Syntax Error at Line %d: Expected parameter name.\n", lines);
                errorCount++;
                recover();
                return;
            }
            
            declareIdentifier(peek().value, paramType);
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
                printf("Syntax Error at Line %d: Identifier '%s' is not declared\n", lines, peek().value);
                errorCount++;
            }
            match("IDENTIFIER");
        }
    } else if (strcmp(peek().type, "LEFT_PAREN") == 0) {
        match("LEFT_PAREN");
        parseExpression();
        if (strcmp(peek().type, "RIGHT_PAREN") != 0) {
            printf("Syntax Error at Line %d: Missing closing parenthesis\n", lines);
            errorCount++;
            recover();
            return;
        }
        match("RIGHT_PAREN");
    } else if (strcmp(peek().type, "RESERVED_WORD") == 0 &&
               (strcmp(peek().value, "True") == 0 || strcmp(peek().value, "False") == 0)) {

        advance();
    } else {
        printf("Syntax Error at Line %d: Missing operand - Expected NUMBER, IDENTIFIER, STRING, or '(' but got %s (%s)\n", lines,
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

        int typeLength = open - line;
        if (typeLength >= sizeof(tokens[tokenCount].type)) {
            typeLength = sizeof(tokens[tokenCount].type) - 1;
        }
        strncpy(tokens[tokenCount].type, line, typeLength);
        tokens[tokenCount].type[typeLength] = '\0';
        if (strcmp(tokens[tokenCount].type, "COMMENT_START") == 0 || strcmp(tokens[tokenCount].type, "COMMENT") == 0 || strcmp(tokens[tokenCount].type, "COM_STR") == 0 || strcmp(tokens[tokenCount].type, "COMMENT_END") == 0) {
            continue;
        }
        if (!open || !close || close < open) {
            continue;
        }

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

    int i = 0;
    while (i < tokenCount && strcmp(tokens[i].type, "NEW_LINE") == 0) {
        lines++;
        i++;
    }
    current = i;

    parseProgram();

    if (errorCount > 0) {
        printf("Parsing finished. Total errors: %d\n", errorCount);
    } else {
        printf("Parsing finished successfully.\n");
    }

    return 0;
}