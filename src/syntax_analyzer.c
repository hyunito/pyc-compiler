#include <stdio.h>
#include <string.h>

#define MAX_TOKENS 999999
#define MAX_IDENTIFIERS 1000
#define TRACK 1

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
    logTransition("errorRecovery", tokens[current].value, "dispatching");
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

void declareIdentifier(const char* name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return;
        }
    }
    if (symbolCount < MAX_IDENTIFIERS) {
        strncpy(symbolTable[symbolCount].name, name, sizeof(symbolTable[symbolCount].name)-1);
        symbolTable[symbolCount].name[sizeof(symbolTable[symbolCount].name)-1] = '\0';
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
    logTransition("parseProgram", tokens[current].value, "dispatching");
    int start = findMain();
    if (start == -1) {
        printf("Syntax Error: No main() function found.\n");
        return;
    }

    current = start;

    if (!match("KEYWORD")) return;
    match("LEFT_PAREN");
    match("RIGHT_PAREN");
    logTransition("parseProgram", tokens[current].value, "parseBlock");
    parseBlock();
}

void parseBlock() {
    logTransition("parseBlock", tokens[current].value, "dispatching");
    if (isNoise("begin")) {
        logTransition("parseBlock","begin", "body");
        match("NOISE_WORD");
    }

    match("LEFT_BRACE");

    while (!isAtEnd() && strcmp(peek().type, "RIGHT_BRACE") != 0) {
        logTransition("parseBlock", tokens[current].value, "parseStatement");
        parseStatement();
    }

    match("RIGHT_BRACE");

    if (isNoise("end")) match("NOISE_WORD");
}

void parseStatement() {
    logTransition("parseStatement", tokens[current].value, "If | While | For | Expression | Output | Input | Function Definition | Block | Declaration | Function call | Array assignment");
    if (isKeyword("if")) {
        logTransition("parseStatement", tokens[current].value, "parseIf");
        parseIf();
        return;
    }
    if (isKeyword("while")) {
        logTransition("parseStatement", tokens[current].value, "parseWhile");
        parseWhile();
        return;
    }

    if (isKeyword("for")) {
        logTransition("parseStatement", tokens[current].value, "parseFor");
        parseFor();
        return;
    }

    if (isKeyword("break")) {
        logTransition("parseStatement", "break", "end");
        match("KEYWORD");
        match("SEMICOLON");
        return;
    }
    if (isKeyword("continue")) {
        logTransition("parseStatement", "continue", "end");
        match("KEYWORD");
        match("SEMICOLON");
        return;
    }

    if (isKeyword("return")) {
        match("KEYWORD");
        logTransition("parseStatement", tokens[current].value, "parseExpression");
        parseExpression();
        match("SEMICOLON");
        return;
    }

    if (isKeyword("output")) {
        logTransition("parseStatement", tokens[current].value, "parseOutput");
        parseOutput();
        return;
    }

    if (strcmp(peek().type, "IDENTIFIER") == 0 &&
        strcmp(peekNext().type, "ASSIGN_OP") == 0 &&
        isKeywordAt(2, "input")) {
        logTransition("parseStatement", tokens[current].value, "parseInput");
        parseInputStatement();
        return;
    }

    if (strcmp(peek().type, "RESERVED_WORD") == 0 &&
        isTypeValue(peek().value) &&
        strcmp(peekNext().type, "IDENTIFIER") == 0 &&
        strcmp(peekAt(2).type, "LEFT_PAREN") == 0) {
        logTransition("parseStatement", tokens[current].value, "parseFunctionDef");
        parseFunctionDef();
        return;
    }

    if (isNoise("begin")) {
        match("NOISE_WORD");
        logTransition("parseStatement", tokens[current].value, "parseBlock");
        parseBlock();
        return;
    }
    if (strcmp(peek().type, "LEFT_BRACE") == 0) {
        logTransition("parseStatement", tokens[current].value, "parseBlock");
        parseBlock();
        return;
    }

    if (strcmp(peek().type, "RESERVED_WORD") == 0 && isTypeValue(peek().value)) {
        logTransition("parseStatement", tokens[current].value, "parseDeclaration");
        parseDeclaration();
        return;
    }

    if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "LEFT_PAREN") == 0) {
        logTransition("parseStatement", tokens[current].value, "parseFunctionCall");
        parseFunctionCall();
        match("SEMICOLON");
        return;
    }

    if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "LEFT_BRACKET") == 0) {
        logTransition("parseStatement", tokens[current].value, "parseArrayAssignment");
        parseArrayAssignment();
        match("SEMICOLON");
        return;
    }

    if (strcmp(peek().type, "INCREMENT") == 0 || strcmp(peek().type, "DECREMENT") == 0) {
        advance();
        logTransition("parseStatement", "IDENTIFIER", "end");
        match("IDENTIFIER");
        match("SEMICOLON");
        return;
    }

    if (strcmp(peek().type, "IDENTIFIER") == 0 &&
       (strcmp(peekNext().type, "INCREMENT") == 0 || strcmp(peekNext().type, "DECREMENT") == 0)) {
        logTransition("parseStatement", "IDENTIFIER", "end");
        match("IDENTIFIER");
        advance();
        match("SEMICOLON");
        return;
    }
    if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "ASSIGN_OP") == 0) {
        logTransition("parseStatement", tokens[current].value, "parseAssignment");
        parseAssignment();
        return;
    }
    if (strcmp(peek().type, "NEW_LINE") == 0) {
        advance();
    }

    printf("Syntax Error at Line %d: Unknown statement starting with %s (%s)\n",lines, peek().type, peek().value);
    errorCount++;
    logTransition("parseStatement", tokens[current].value, "errorRecovery");
    recover();
}

void parseDeclaration() {
    match("RESERVED_WORD");
    logTransition("parseDeclaration", tokens[current].value, "ASSIGN_OP | COMMA");

    if (strcmp(peek().type, "IDENTIFIER") != 0) {
        printf("Syntax Error at Line %d: Expected identifier in declaration.\n", lines);
        errorCount++;
        logTransition("parseDeclaration", tokens[current].value, "errorRecovery");
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
            logTransition("ASSIGN_OP", tokens[current].value, "parseExpression");
            parseExpression();
        }

        declareIdentifier(idname);

        if (strcmp(peek().type, "COMMA") == 0) {
            match("COMMA");
            logTransition("COMMA", tokens[current].value, "parseDeclaration");
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
    logTransition("condition", tokens[current].value, "parseExpression");
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
    logTransition("parseIf", "if", "condition");
    if (strcmp(peek().type, "KEYWORD") != 0 || strcmp(peek().value, "if") != 0) {
        printf("Internal Error: parseIf called but current token is %s (%s)\n", peek().type, peek().value);
        errorCount++;
        logTransition("parseIf", tokens[current].value, "errorRecovery");
        recover();
        return;
    }
    match("KEYWORD");
    logTransition("parseIf", "condition", "parseExpression");
    if (!expectCondition()) {
        if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
            logTransition("parseIf", tokens[current].value, "parseBlock");
            parseBlock();
        } else {
            logTransition("parseIf", tokens[current].value, "parseStatement");
            parseStatement();
        }
        return;
    }

    if (isNoise("then")) match("NOISE_WORD");
    logTransition("condition", tokens[current].value, "Block | Statement");
    if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
        logTransition("parseIf", tokens[current].value, "parseBlock");
        parseBlock();
    } else {
        logTransition("parseIf", tokens[current].value, "parseStatement");
        parseStatement();
    }

    while (isKeyword("else")) {
        match("KEYWORD");

        if (isKeyword("if")) {
            match("KEYWORD");
            logTransition("parseIf", "else if condition", "parseExpression");
            if (!expectCondition()) {
                if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
                    logTransition("parseIf", "condition", "parseBlock");
                    parseBlock();
                } else {
                    logTransition("parseIf", "condition", "parseStatement");
                    parseStatement();
                }
                break;
            }
            if (isNoise("then")) match("NOISE_WORD");

            if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
                logTransition("parseIf", tokens[current].value, "parseBlock");
                parseBlock();
            } else {
                logTransition("parseIf", tokens[current].value, "parseStatement");
                parseStatement();
            }
        } else {
            logTransition("parseIf", "else", "Block | Statement");
            if (isNoise("then")) match("NOISE_WORD");
            if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
                logTransition("parseIf", tokens[current].value, "parseBlock");
                parseBlock();
            } else {
                logTransition("parseIf", tokens[current].value, "parseStatement");
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
        logTransition("parseWhile", tokens[current].value, "errorRecovery");
        recover();
        return;
    }
    match("KEYWORD");
    logTransition("parseWhile", "condition", "parseExpression");
    if (!expectCondition()) {
        if (isNoise("do")) match("NOISE_WORD");
        if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
            logTransition("parseWhile", tokens[current].value, "parseBlock");
            parseBlock();
        } else {
            logTransition("parseWhile", tokens[current].value, "parseStatement");
            parseStatement();
        }
        return;
    }
    logTransition("parseWhile", "condition", "body");
    if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin") || isNoise("do")) {
        if (isNoise("do")) match("NOISE_WORD");
        logTransition("parseWhile", tokens[current].value, "parseBlock");
        parseBlock();
    } else {
        logTransition("parseWhile", tokens[current].value, "parseStatement");
        parseStatement();
    }
}

void parseFor() {
    logTransition("parseFor", tokens[current].value, "dispatching");
    if (strcmp(peek().type, "KEYWORD") != 0 || strcmp(peek().value, "for") != 0) {
        printf("Internal Error: parseFor called but current token is %s (%s)\n", peek().type, peek().value);
        errorCount++;
        logTransition("parseFor", tokens[current].value, "errorRecovery");
        recover();
        return;
    }
    match("KEYWORD");

    if (strcmp(peek().type, "LEFT_PAREN") != 0) {
        printf("Syntax Error at Line %d: Expected '(' after for but got %s (%s)\n", lines, peek().type, peek().value);
        errorCount++;
        logTransition("parseFor", tokens[current].value, "errorRecovery");
        recover();
    } else {
        match("LEFT_PAREN");
        logTransition("parseFor", "condition", "Semicolon | Declaration | Expression | Assignment");
    }

    int initConsumedSemicolon = 0;
    if (strcmp(peek().type, "SEMICOLON") == 0) {
        match("SEMICOLON");
        initConsumedSemicolon = 1;
    } else if (strcmp(peek().type, "RESERVED_WORD") == 0 && isTypeValue(peek().value)) {
        logTransition("parseFor", tokens[current].value, "parseDeclaration");
        parseDeclaration();
        initConsumedSemicolon = 1;
    } else {
        if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "ASSIGN_OP") == 0) {
            logTransition("parseFor", tokens[current].value, "parseAssignment");
            parseAssignment();
            initConsumedSemicolon = 1;
        } else {
            logTransition("parseFor", "condition", "parseExpression");
            parseExpression();
            if (strcmp(peek().type, "SEMICOLON") == 0) {
                match("SEMICOLON");
                initConsumedSemicolon = 1;
            } else {
                printf("Syntax Error at Line %d: Expected ';' after for-initialization but got %s (%s)\n", lines,
                       peek().type, peek().value);
                errorCount++;
                logTransition("parseFor", tokens[current].value, "errorRecovery");
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
            logTransition("parseFor", tokens[current].value, "errorRecovery");
            recover();
        }
    }

    if (strcmp(peek().type, "SEMICOLON") == 0) {
        match("SEMICOLON");
    } else {
        logTransition("parseFor", tokens[current].value, "parseExpression");
        parseExpression();
        if (strcmp(peek().type, "SEMICOLON") == 0) {
            match("SEMICOLON");
        } else {
            printf("Syntax Error at Line %d: Expected ';' after for-condition but got %s (%s)\n", lines,
                   peek().type, peek().value);
            errorCount++;
            logTransition("parseFor", tokens[current].value, "errorRecovery");
            recover();
        }
    }


    if (strcmp(peek().type, "RIGHT_PAREN") == 0) {
        logTransition("parseFor", "condition", "body");
    } else {
        if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "ASSIGN_OP") == 0) {
            char namebuf[100];
            strncpy(namebuf, peek().value, sizeof(namebuf)-1);
            namebuf[sizeof(namebuf)-1] = '\0';
            match("IDENTIFIER");
            match("ASSIGN_OP");
            logTransition("parseFor", tokens[current].value, "parseExpression");
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
            logTransition("parseFor", tokens[current].value, "parseExpression");
            parseExpression();
        }
    }

    if (strcmp(peek().type, "RIGHT_PAREN") == 0) {
        match("RIGHT_PAREN");
        logTransition("parseFor", "condition", "body");
    } else {
        printf("Syntax Error at Line %d: Expected ')' to close for loop header but got %s (%s)\n", lines,
               peek().type, peek().value);
        errorCount++;
        logTransition("parseFor", tokens[current].value, "errorRecovery");
        recover();
    }


    if (isNoise("do")) match("NOISE_WORD");

    if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
        logTransition("parseFor", tokens[current].value, "parseBlock");
        parseBlock();
    } else {
        logTransition("parseFor", tokens[current].value, "parseStatement");
        parseStatement();
    }
}

void parseAssignment() {
    logTransition("parseAssignment", tokens[current].value, "dispatching");
    if (strcmp(peek().type, "IDENTIFIER") != 0) {
        printf("Syntax Error at Line %d: Assignment must start with identifier.\n", lines);
        errorCount++;
        logTransition("parseAssignment", tokens[current].value, "errorRecovery");
        recover();
        return;
    }
    char namebuf[100];
    strncpy(namebuf, peek().value, sizeof(namebuf)-1);
    namebuf[sizeof(namebuf)-1] = '\0';
    logTransition("parseAssignment", "identifier", "assign_op");
    match("IDENTIFIER");
    match("ASSIGN_OP");


    if (strcmp(peek().type, "KEYWORD") == 0 && strcmp(peek().value, "input") == 0) {
        logTransition("parseAssignment", "input", "string");
        match("KEYWORD");
        match("LEFT_PAREN");
        match("STRING");
        match("RIGHT_PAREN");
    } else {
        logTransition("parseAssignment", tokens[current].value, "parseExpression");
        parseExpression();
    }

    match("SEMICOLON");

    if (!isIdentifierDeclared(namebuf)) {
        printf("Warning: Identifier '%s' assigned but not declared.\n", namebuf);
    }
}

void parseArrayAssignment() {
    logTransition("parseArrayAssignment", tokens[current].value, "dispatching");

    match("IDENTIFIER");
    match("LEFT_BRACKET");

    if (strcmp(peek().type, "RIGHT_BRACKET") != 0) {
        logTransition("parseArrayAssignment", tokens[current].value, "parseExpression");
        parseExpression();
    }
    match("RIGHT_BRACKET");

    match("ASSIGN_OP");

    if (strcmp(peek().type, "LEFT_BRACE") == 0) {
        match("LEFT_BRACE");
        if (strcmp(peek().type, "RIGHT_BRACE") != 0) {
            logTransition("parseArrayAssignment", tokens[current].value, "parseExpression");
            parseExpression();
            while (strcmp(peek().type, "COMMA") == 0) {
                match("COMMA");
                logTransition("parseArrayAssignment", tokens[current].value, "parseExpression");
                parseExpression();
            }
        }
        match("RIGHT_BRACE");
    } else {
        logTransition("parseArrayAssignment", tokens[current].value, "parseExpression");
        parseExpression();
    }
}

void parseInputStatement() {
    logTransition("parseInput", tokens[current].value, "dispatching");
    if (strcmp(peek().type, "IDENTIFIER") != 0) {
        printf("Syntax Error at Line %d: input assignment must start with identifier.\n", lines);
        errorCount++;
        logTransition("parseInputStatement", tokens[current].value, "errorRecovery");
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
    logTransition("parseOutput", tokens[current].value, "dispatching");
    match("KEYWORD");
    match("LEFT_PAREN");

    if (strcmp(peek().type, "RIGHT_PAREN") != 0) {
        if (strcmp(peek().type, "STRING") == 0) {
            match("STRING");
        } else {
            logTransition("parseOutputStatement", tokens[current].value, "parseExpression");
            parseExpression();
        }

        while (strcmp(peek().type, "COMMA") == 0) {
            match("COMMA");
            if (strcmp(peek().type, "STRING") == 0) {
                match("STRING");
            } else {
                logTransition("parseOutputStatement", tokens[current].value, "parseExpression");
                parseExpression();
            }
        }
    }

    match("RIGHT_PAREN");
    match("SEMICOLON");
}

void parseFunctionCall() {
    logTransition("parseFunctionCall", tokens[current].value, "dispatching");
    match("IDENTIFIER");
    match("LEFT_PAREN");
    if (strcmp(peek().type, "RIGHT_PAREN") != 0) {
        logTransition("parseFunctionCall", tokens[current].value, "parseExpression");
        parseExpression();
        while (strcmp(peek().type, "COMMA") == 0) {
            match("COMMA");
            logTransition("parseFunctionCall", tokens[current].value, "parseExpression");
            parseExpression();
        }
    }
    match("RIGHT_PAREN");
}
//checkpoint
void parseFunctionDef() {
    logTransition("parseFunctionDef", tokens[current].value, "dispatching");
    match("RESERVED_WORD");
    if (strcmp(peek().type, "IDENTIFIER") != 0) {
        printf("Syntax Error at Line %d: Expected function name.\n", lines);
        errorCount++;
        logTransition("parseFunctionDef", tokens[current].value, "errorRecovery");
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
                logTransition("parseFunctionDef", tokens[current].value, "errorRecovery");
                recover();
                return;
            }
            match("RESERVED_WORD");
            if (strcmp(peek().type, "IDENTIFIER") != 0) {
                printf("Syntax Error at Line %d: Expected parameter name.\n", lines);
                errorCount++;
                logTransition("parseFunctionDef", tokens[current].value, "errorRecovery");
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

    logTransition("parseFunctionDef", tokens[current].value, "parseBlock");
    parseBlock();
}

void parseExpression() {
    logTransition("parseExpression", tokens[current].value, "dispatching");
    logTransition("parseExpression", tokens[current].value, "parseLogicalOr");
    parseLogicalOr();
}

void parseLogicalOr() {
    logTransition("parseLogicalOr", tokens[current].value, "dispatching");
    logTransition("parseLogicalOr", tokens[current].value, "parseLogicalAnd");
    parseLogicalAnd();
    while (strcmp(peek().type, "LOGICAL") == 0 && strcmp(peek().value, "or") == 0) {
        match("LOGICAL");
        logTransition("parseLogicalOr", tokens[current].value, "parseLogicalAnd");
        parseLogicalAnd();
    }
}

void parseLogicalAnd() {
    logTransition("parseLogicalAnd", tokens[current].value, "dispatching");
    logTransition("parseLogicalAnd", tokens[current].value, "parseLogicalNot");
    parseLogicalNot();
    while (strcmp(peek().type, "LOGICAL") == 0 && strcmp(peek().value, "and") == 0) {
        match("LOGICAL");
        logTransition("parseLogicalAnd", tokens[current].value, "parseLogicalNot");
        parseLogicalNot();
    }
}

void parseLogicalNot() {
    logTransition("parseLogicalNot", tokens[current].value, "dispatching");
    if (strcmp(peek().type, "LOGICAL") == 0 && strcmp(peek().value, "not") == 0) {
        match("LOGICAL");
        logTransition("parseLogicalNot", tokens[current].value, "parseLogicalNot");
        parseLogicalNot();
        return;
    }
    logTransition("parseLogicalNot", tokens[current].value, "parseRelational");
    parseRelational();
}

void parseRelational() {
    logTransition("parseRelational", tokens[current].value, "dispatching");
    logTransition("parseRelational", tokens[current].value, "parseArithmetic");
    parseArithmetic();
    while (strcmp(peek().type, "REL_OP") == 0) {
        match("REL_OP");
        logTransition("parseRelational", tokens[current].value, "parseArithmetic");
        parseArithmetic();
    }
}

void parseArithmetic() {
    logTransition("parseArithmetic", tokens[current].value, "dispatching");
    logTransition("parseArithmetic", tokens[current].value, "parseTerm");
    parseTerm();
    while (strcmp(peek().type, "ARITH_OP") == 0 &&
          (strcmp(peek().value, "+") == 0 || strcmp(peek().value, "-") == 0)) {
        match("ARITH_OP");
        logTransition("parseArithmetic", tokens[current].value, "parseTerm");
        parseTerm();
    }
}

void parseTerm() {
    logTransition("parseTerm", tokens[current].value, "dispatching");
    logTransition("parseTerm", tokens[current].value, "parseFactor");
    parseFactor();
    while (strcmp(peek().type, "ARITH_OP") == 0 &&
          (strcmp(peek().value, "*") == 0 ||
           strcmp(peek().value, "/") == 0 ||
           strcmp(peek().value, "%") == 0 ||
           strcmp(peek().value, "//") == 0)) {
        match("ARITH_OP");
        logTransition("parseTerm", tokens[current].value, "parseFactor");
        parseFactor();
    }
}

void parseFactor() {
    logTransition("parseFactor", tokens[current].value, "dispatching");
    if (strcmp(peek().type, "ARITH_OP") == 0 &&
        (strcmp(peek().value, "+") == 0 || strcmp(peek().value, "-") == 0)) {
        match("ARITH_OP");
        logTransition("parseFactor", tokens[current].value, "parseFactor");
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
        logTransition("parseFactor", tokens[current].value, "parseExpression");
        parseExpression();
        if (strcmp(peek().type, "RIGHT_PAREN") != 0) {
            printf("Syntax Error at Line %d: Missing closing parenthesis\n", lines);
            errorCount++;
            logTransition("parseFactor", tokens[current].value, "errorRecovery");
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
        logTransition("parseFactor", tokens[current].value, "errorRecovery");
        recover();
        return;
    }
    if (strcmp(peek().type, "ARITH_OP") == 0 && strcmp(peek().value, "**") == 0) {
        match("ARITH_OP");
        logTransition("parseFactor", tokens[current].value, "parseFactor");
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
        printf("TESTING TOKEN COUNT\n");
        lines++;
        i++;
    }
    current = i;

    parseProgram();

    if (errorCount > 0) {
        printf("Parsing finished. Total errors: %d\n", errorCount);
    }

    return 0;
}