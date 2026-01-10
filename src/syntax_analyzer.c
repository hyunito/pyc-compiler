#include <stdio.h>
#include <string.h>


#define MAX_IDENTIFIERS 1000
#define TRACK 1
#define TRACK1 1

typedef struct {
    char type[50];
    char value[100];
} Token;

typedef struct {
    char name[100];
    char type[20];
    int isDeclared;
    char value[256];   // <-- ADD THIS
} Identifier;

Token peek();
Token peekNext();
Token peekAt(int offset);
void advance();
int isAtEnd();
void recover();

Token getNextToken();

Token lookahead[3];
void initLookahead() {
    for (int i = 0; i < 3; i++) {
        lookahead[i] = getNextToken();
    }
}
int tokenCount = 0;

int errorCount = 0;
int lines = 1;
int braceBalance = 0;

Identifier symbolTable[MAX_IDENTIFIERS];
int symbolCount = 0;

static Token EOF_TOKEN = { "EOF", "" };

int isAtEnd() {
    return strcmp(peek().type, "EOF") == 0;
}

Token peekAt(int i);

Token peek() {
    return lookahead[0];

}

Token peekAt(int offset) {
    if (offset >= 0 && offset < 3)
        return lookahead[offset];
    return EOF_TOKEN;
}

Token peekNext() {  return lookahead[1]; }


const char* getIdentifierValue(const char* name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            if (symbolTable[i].value[0] == '\0')
                return NULL;
            return symbolTable[i].value;
        }
    }
    return NULL;
}


void logTransition(const char* from, const char* input, const char* to) {
    if (TRACK) printf("%s --%s--> %s\n", from, input, to);
}

void advance() {
    lookahead[0] = lookahead[1];
    lookahead[1] = lookahead[2];
    lookahead[2] = getNextToken();

}

int match(const char* expected) {
    if (strcmp(peek().type, expected) == 0) {
        if (strcmp(expected, "LEFT_BRACE") == 0) braceBalance++;
        if (strcmp(expected, "RIGHT_BRACE") == 0) {
            braceBalance--;
            if (braceBalance < 0) {
                printf("Syntax Error at Line %d: Extra closing brace '}'.\n", lines);
                errorCount++;
                braceBalance = 0;
            }
        }

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
            printf("Syntax Error at Line %d: Identifier '%s' is already declared.\n", lines, name);
            errorCount++;
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

int isConstTypeStart() {
    return strcmp(peek().type, "KEYWORD") == 0 &&
           strcmp(peek().value, "const") == 0;
}

void recover() {
    logTransition("errorRecovery", peek().value, "dispatching");
    if (isAtEnd()) return;

    advance();

    while (!isAtEnd()) {

        if (strcmp(peek().type, "SEMICOLON") == 0) {
            advance();
            return;
        }

        if (strcmp(peek().type, "RIGHT_BRACE") == 0) {
            return;
        }

        if (strcmp(peek().type, "RESERVED_WORD") == 0 &&
            isTypeValue(peek().value)) {
            return;
            }

        if (strcmp(peek().type, "KEYWORD") == 0) {
            if (strcmp(peek().value, "if") == 0 ||
                strcmp(peek().value, "for") == 0 ||
                strcmp(peek().value, "while") == 0 ||
                strcmp(peek().value, "return") == 0 ||
                strcmp(peek().value, "output") == 0 ||
                strcmp(peek().value, "input") == 0) {
                return;
                }
        }

        if (strcmp(peek().type, "IDENTIFIER") == 0) {
            return;
        }

        advance();
    }
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
    logTransition("parseProgram", peek().value, "Finding main()");
    if (!isKeyword("main")) {
        printf("Syntax Error: Program must start with main()\n");
        return;
    }
    match("KEYWORD");
    match("LEFT_PAREN");
    match("RIGHT_PAREN");

    logTransition("parseProgram", peek().value, "parseBlock");
    parseBlock();
    if (TRACK1) printf("parseBlock: DONE\n");
}

void parseBlock() {
    int hasBegin = 0;
    int hasBrace = 0;
    logTransition("parseBlock", peek().value, "dispatching");
    if (isNoise("begin")) {
        logTransition("parseBlock","begin", "body");
        match("NOISE_WORD");
        hasBegin = 1;
    }

    if (strcmp(peek().type, "LEFT_BRACE") == 0) {
        match("LEFT_BRACE");
        hasBrace = 1;
    }

    if (!hasBegin && !hasBrace) {
        printf("Syntax Error at Line %d: Expected '{' or 'begin' to start block but got %s (%s)\n",
               lines, peek().type, peek().value);
        errorCount++;
        recover();
        return;
    }

    while (!isAtEnd()) {
        if (hasBrace && strcmp(peek().type, "RIGHT_BRACE") == 0) {
            break;
        }
        if (hasBegin && isNoise("end")) {
            break;
        }
        logTransition("parseBlock", peek().value, "parseStatement");
        parseStatement();
        if (TRACK1) printf("parseStatement: DONE\n");
    }

    if (hasBrace) {
        match("RIGHT_BRACE");
    }

    if (hasBegin && isNoise("end")) {
        match("NOISE_WORD");

        if (strcmp(peek().type, "SEMICOLON") == 0) {
            match("SEMICOLON");
        }
    }
}

void parseStatement() {
    logTransition("parseStatement", peek().value, "If | While | For | Expression | Output | Input | Function Definition | Block | Declaration | Function call | Array assignment");
    if (isKeyword("if")) {
        logTransition("parseStatement", peek().value, "parseIf");
        parseIf();
        if (TRACK1) printf("parseIf: DONE\n");
        return;
    }
    if (isKeyword("while")) {
        logTransition("parseStatement", peek().value, "parseWhile");
        parseWhile();
        if (TRACK1) printf("parseWhile: DONE\n");
        return;
    }

    if (isKeyword("for")) {
        logTransition("parseStatement", peek().value, "parseFor");
        parseFor();
        if (TRACK1) printf("parseFor: DONE\n");
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
        if (strcmp(peek(). type, "SEMICOLON") != 0) {
            logTransition("parseStatement", peek().value, "parseExpression");
            parseExpression();
            if (TRACK1) printf("parseExpression: DONE\n");
        }
        match("SEMICOLON");
        return;
    }

    if (isKeyword("output")) {
        logTransition("parseStatement", peek().value, "parseOutput");
        parseOutput();
        if (TRACK1) printf("parseOutput: DONE\n");
        return;
    }

    if (strcmp(peek().type, "IDENTIFIER") == 0 &&
        strcmp(peekNext().type, "ASSIGN_OP") == 0 &&
        isKeywordAt(2, "input")) {
        logTransition("parseStatement", peek().value, "parseInput");
        parseInputStatement();
        if (TRACK1) printf("parseInput: DONE\n");
        return;
    }

    if (strcmp(peek().type, "RESERVED_WORD") == 0 &&
        isTypeValue(peek().value) &&
        strcmp(peekNext().type, "IDENTIFIER") == 0 &&
        strcmp(peekAt(2).type, "LEFT_PAREN") == 0) {
        logTransition("parseStatement", peek().value, "parseFunctionDef");
        parseFunctionDef();
        if (TRACK1) printf("parseFunctionDef: DONE\n");
        return;
    }

    if (isNoise("begin")) {
        match("NOISE_WORD");
        logTransition("parseStatement", peek().value, "parseBlock");
        parseBlock();
        if (TRACK1) printf("parseBlock: DONE\n");
        return;
    }
    if (strcmp(peek().type, "LEFT_BRACE") == 0) {
        logTransition("parseStatement", peek().value, "parseBlock");
        parseBlock();
        if (TRACK1) printf("parseBlock: DONE\n");
        return;
    }

    if (isConstTypeStart()) {
        logTransition("parseStatement", peek().value, "parseDeclaration");
        parseDeclaration();
        if (TRACK1) printf("parseDeclaration: DONE\n");
        return;
    }

    if (strcmp(peek().type, "RESERVED_WORD") == 0 && isTypeValue(peek().value)) {
        logTransition("parseStatement", peek().value, "parseDeclaration");
        parseDeclaration();
        if (TRACK1) printf("parseDeclaration: DONE\n");
        return;
    }

    if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "LEFT_PAREN") == 0) {
        logTransition("parseStatement", peek().value, "parseFunctionCall");
        parseFunctionCall();
        if (TRACK1) printf("parseFunctionCall: DONE\n");
        match("SEMICOLON");
        return;
    }

    if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "LEFT_BRACKET") == 0) {
        logTransition("parseStatement", peek().value, "parseArrayAssignment");
        parseArrayAssignment();
        if (TRACK1) printf("parseArrayAssignment: DONE\n");
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
        logTransition("parseStatement", peek().value, "parseAssignment");
        if (TRACK1) printf("parseAssignment: DONE\n");
        parseAssignment();
        return;
    }
    if (strcmp(peek().type, "NEW_LINE") == 0) {
        advance();
    }

    printf("Syntax Error at Line %d: Unknown statement starting with %s (%s)\n",lines, peek().type, peek().value);
    errorCount++;
    logTransition("parseStatement", peek().value, "errorRecovery");
    recover();
}

void parseDeclaration() {
    int isConst = 0;

    if (isConstTypeStart()) {
        isConst = 1;
        match("KEYWORD");
    }

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

    if (strcmp(peekAt(1).type, "LEFT_BRACKET") == 0) {
        logTransition("parseDeclaration", peek().value, "parseArrayAssignment");
        char idname[100];
        strncpy(idname, peek().value, sizeof(idname)-1);
        idname[sizeof(idname)-1] = '\0';
        declareIdentifier(idname, declaredType);
        parseArrayAssignment();
        if (TRACK1) printf("parseArrayAssignment: DONE\n");
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
                logTransition("ASSIGN_OP", peek().value, "parseExpression");
                parseExpression();
                if (TRACK1) printf("parseExpression: DONE\n");
            }
        }

        if (strcmp(peek().type, "COMMA") == 0) {
            match("COMMA");
            logTransition("COMMA", peek().value, "parseDeclaration");
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
    logTransition("parseIf", "if", "condition");
    if (strcmp(peek().type, "KEYWORD") != 0 || strcmp(peek().value, "if") != 0) {
        printf("Internal Error: parseIf called but current token is %s (%s)\n", peek().type, peek().value);
        errorCount++;
        logTransition("parseIf", peek().value, "errorRecovery");
        recover();
        return;
    }
    match("KEYWORD");
    logTransition("parseIf", "condition", "parseExpression");
    if (!expectCondition()) {
        if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
            logTransition("parseIf", peek().value, "parseBlock");
            parseBlock();
            if (TRACK1) printf("parseBlock: DONE\n");
        } else {
            logTransition("parseIf", peek().value, "parseStatement");
            parseStatement();
            if (TRACK1) printf("parseStatement: DONE\n");
        }
        return;
    }

    if (isNoise("then")) match("NOISE_WORD");
    logTransition("condition", peek().value, "Block | Statement");
    if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
        logTransition("parseIf", peek().value, "parseBlock");
        parseBlock();
        if (TRACK1) printf("parseBlock: DONE\n");
    } else {
        logTransition("parseIf", peek().value, "parseStatement");
        parseStatement();
        if (TRACK1) printf("parseStatement: DONE\n");
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
                    if (TRACK1) printf("parseBlock: DONE\n");
                } else {
                    logTransition("parseIf", "condition", "parseStatement");
                    parseStatement();
                    if (TRACK1) printf("parseStatement: DONE\n");
                }
                break;
            }
            if (isNoise("then")) match("NOISE_WORD");

            if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
                logTransition("parseIf", peek().value, "parseBlock");
                parseBlock();
                if (TRACK1) printf("parseBlock: DONE\n");
            } else {
                logTransition("parseIf", peek().value, "parseStatement");
                parseStatement();
                if (TRACK1) printf("parseStatement: DONE\n");
            }
        } else {
            logTransition("parseIf", "else", "Block | Statement");
            if (isNoise("then")) match("NOISE_WORD");
            if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
                logTransition("parseIf", peek().value, "parseBlock");
                parseBlock();
                if (TRACK1) printf("parseBlock: DONE\n");
            } else {
                logTransition("parseIf", peek().value, "parseStatement");
                parseStatement();
                if (TRACK1) printf("parseStatement: DONE\n");
            }
            break;
        }
    }
}

void parseWhile() {
    if (strcmp(peek().type, "KEYWORD") != 0 || strcmp(peek().value, "while") != 0) {
        printf("Internal Error: parseWhile called but current token is %s (%s)\n", peek().type, peek().value);
        errorCount++;
        logTransition("parseWhile", peek().value, "errorRecovery");
        recover();
        return;
    }
    match("KEYWORD");
    logTransition("parseWhile", "condition", "parseExpression");
    if (!expectCondition()) {
        if (isNoise("do")) match("NOISE_WORD");
        if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
            logTransition("parseWhile", peek().value, "parseBlock");
            parseBlock();
            if (TRACK1) printf("parseBlock: DONE\n");
        } else {
            logTransition("parseWhile", peek().value, "parseStatement");
            parseStatement();
            if (TRACK1) printf("parseStatement: DONE\n");
        }
        return;
    }
    logTransition("parseWhile", "condition", "body");
    if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin") || isNoise("do")) {
        if (isNoise("do")) match("NOISE_WORD");
        logTransition("parseWhile", peek().value, "parseBlock");
        parseBlock();
        if (TRACK1) printf("parseBlock: DONE\n");
    } else {
        logTransition("parseWhile", peek().value, "parseStatement");
        parseStatement();
        if (TRACK1) printf("parseStatement: DONE\n");
    }
}

void parseFor() {
    logTransition("parseFor", peek().value, "dispatching");
    if (strcmp(peek().type, "KEYWORD") != 0 || strcmp(peek().value, "for") != 0) {
        printf("Internal Error: parseFor called but current token is %s (%s)\n", peek().type, peek().value);
        errorCount++;
        logTransition("parseFor", peek().value, "errorRecovery");
        recover();
        return;
    }
    match("KEYWORD");

    if (strcmp(peek().type, "LEFT_PAREN") != 0) {
        printf("Syntax Error at Line %d: Expected '(' after for but got %s (%s)\n", lines, peek().type, peek().value);
        errorCount++;
        logTransition("parseFor", peek().value, "errorRecovery");
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
        logTransition("parseFor", peek().value, "parseDeclaration");
        parseDeclaration();
        if (TRACK1) printf("parseDeclaration: DONE\n");
        initConsumedSemicolon = 1;
    } else {
        if (strcmp(peek().type, "IDENTIFIER") == 0 && strcmp(peekNext().type, "ASSIGN_OP") == 0) {
            logTransition("parseFor", peek().value, "parseAssignment");
            parseAssignment();
            if (TRACK1) printf("parseAssigment: DONE\n");
            initConsumedSemicolon = 1;
        } else {
            logTransition("parseFor", "condition", "parseExpression");
            parseExpression();
            if (TRACK1) printf("parseExpression: DONE\n");
            if (strcmp(peek().type, "SEMICOLON") == 0) {
                match("SEMICOLON");
                initConsumedSemicolon = 1;
            } else {
                printf("Syntax Error at Line %d: Expected ';' after for-initialization but got %s (%s)\n", lines,
                       peek().type, peek().value);
                errorCount++;
                logTransition("parseFor", peek().value, "errorRecovery");
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
            logTransition("parseFor", peek().value, "errorRecovery");
            recover();
        }
    }

    if (strcmp(peek().type, "SEMICOLON") == 0) {
        match("SEMICOLON");
    } else {
        logTransition("parseFor", peek().value, "parseExpression");
        parseExpression();
        if (strcmp(peek().type, "SEMICOLON") == 0) {
            match("SEMICOLON");
        } else {
            printf("Syntax Error at Line %d: Expected ';' after for-condition but got %s (%s)\n", lines,
                   peek().type, peek().value);
            errorCount++;
            logTransition("parseFor", peek().value, "errorRecovery");
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
            logTransition("parseFor", peek().value, "parseExpression");
            parseExpression();
            if (TRACK1) printf("parseExpression: DONE\n");
            if (TRACK1) printf("parseExpression: DONE\n");
            if (!isIdentifierDeclared(namebuf)) {
                errorCount++;
                printf("Syntax Error at Line %d: Identifier '%s' assigned but not declared.\n", lines, namebuf);
            }
        } else if (strcmp(peek().type, "IDENTIFIER") == 0 &&
                   (strcmp(peekNext().type, "INCREMENT") == 0 || strcmp(peekNext().type, "DECREMENT") == 0)) {
            match("IDENTIFIER");
            advance();
        } else if (strcmp(peek().type, "INCREMENT") == 0 || strcmp(peek().type, "DECREMENT") == 0) {
            advance();
            match("IDENTIFIER");
        } else {
            logTransition("parseFor", peek().value, "parseExpression");
            parseExpression();
            if (TRACK1) printf("parseExpression: DONE\n");
        }
    }

    if (strcmp(peek().type, "RIGHT_PAREN") == 0) {
        match("RIGHT_PAREN");
        logTransition("parseFor", "condition", "body");
    } else {
        printf("Syntax Error at Line %d: Expected ')' to close for loop header but got %s (%s)\n", lines,
               peek().type, peek().value);
        errorCount++;
        logTransition("parseFor", peek().value, "errorRecovery");
        recover();
    }


    if (isNoise("do")) match("NOISE_WORD");

    if (strcmp(peek().type, "LEFT_BRACE") == 0 || isNoise("begin")) {
        logTransition("parseFor", peek().value, "parseBlock");
        parseBlock();
        if (TRACK1) printf("parseBlock: DONE\n");
    } else {
        logTransition("parseFor", peek().value, "parseStatement");
        parseStatement();
        if (TRACK1) printf("parseStatement: DONE\n");
    }
}

void parseAssignment() {
    logTransition("parseAssignment", peek().value, "dispatching");
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
        logTransition("parseAssignment", peek().value, "parseExpression");
        parseExpression();
        if (TRACK1) printf("parseExpression: DONE\n");
    }

    match("SEMICOLON");

    if (!isIdentifierDeclared(namebuf)) {
        errorCount++;
        printf("Syntax Error at Line %d: Identifier '%s' assigned but not declared.\n", lines, namebuf);
    }
}

void parseArrayAssignment() {
    logTransition("parseArrayAssignment", peek().value, "dispatching");

    if (strcmp(peek().type, "IDENTIFIER") == 0) match("IDENTIFIER");

    match("LEFT_BRACKET");

    if (strcmp(peek().type, "RIGHT_BRACKET") != 0) {
        logTransition("parseArrayAssignment", peek().value, "parseExpression");
        parseExpression();
    }
    match("RIGHT_BRACKET");

    match("ASSIGN_OP");

    if (strcmp(peek().type, "LEFT_BRACE") == 0) {
        match("LEFT_BRACE");
        if (strcmp(peek().type, "RIGHT_BRACE") != 0) {
            logTransition("parseArrayAssignment", peek().value, "parseExpression");
            parseExpression();
            if (TRACK1) printf("parseExpression: DONE\n");
            while (strcmp(peek().type, "COMMA") == 0) {
                match("COMMA");
                logTransition("parseArrayAssignment", peek().value, "parseExpression");
                parseExpression();
                if (TRACK1) printf("parseExpression: DONE\n");
            }
        }
        match("RIGHT_BRACE");
    } else {
        logTransition("parseArrayAssignment", peek().value, "parseExpression");
        parseExpression();
        if (TRACK1) printf("parseExpression: DONE\n");
    }
    match("SEMICOLON");
}

void parseInputStatement() {
    logTransition("parseInput", peek().value, "dispatching");
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
        printf("Semantic Error at Line %d: Identifier '%s' is not declared.\n", lines, namebuf);
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
    if (strcmp(peek().type, "STRING") == 0)
        match("STRING");
    else if (strcmp(peek().type, "STRING_INTERP") == 0)
        match("STRING_INTERP");
    else {
        printf("Syntax Error at Line %d: input prompt must be a string literal.\n", lines);
        errorCount++;
        recover();
        return;
    }
    match("RIGHT_PAREN");
    match("SEMICOLON");
}

void parseOutput() {
    logTransition("parseOutput", peek().value, "dispatching");

    match("KEYWORD");
    match("LEFT_PAREN");

    if (strcmp(peek().type, "STRING") != 0 && strcmp(peek().type, "STRING_INTERP") != 0) {
        errorCount++;
        printf("Syntax Error at Line %d: Output statement must start with a string literal.\n", lines);
        logTransition("parseOutput", peek().value, "errorRecovery");
        recover();
        return;
    }

    validateStringInterpolation(peek().value);
    if (strcmp(peek().type, "STRING_INTERP") == 0) {
        char temp[256];
        strcpy(temp, peek().value);
        char *start = strchr(temp, '{');
        char *end   = strchr(temp, '}');

        if (start && end && end > start) {

            char varname[100];
            strncpy(varname, start + 1, end - start - 1);
            varname[end - start - 1] = '\0';

            const char *val = getIdentifierValue(varname);
            if (!val) val = "(undefined)";

            char finalOut[256];
            int prefixLen = start - temp;

            strncpy(finalOut, temp, prefixLen);
            finalOut[prefixLen] = '\0';
            strcat(finalOut, val);
            strcat(finalOut, end + 1);

            //if (TRACK1) printf("%s\n", finalOut);
        } else {
            // No valid interpolation
            //if (TRACK1) printf("%s\n", temp);
        }

        match("STRING_INTERP");
    }
    else {
        // NORMAL STRING (no interpolation)
        //if (TRACK1) printf("%s\n", peek().value);
        match("STRING");
    }



    if (strcmp(peek().type, "COMMA") == 0) {
        errorCount++;
        printf("Syntax Error at Line %d: Output statement cannot accept multiple arguments.\n", lines);
        logTransition("parseOutput", peek().value, "errorRecovery");
        recover();
        return;
    }

    match("RIGHT_PAREN");
    match("SEMICOLON");
}

void parseFunctionCall() {
    logTransition("parseFunctionCall", peek().value, "dispatching");
    match("IDENTIFIER");
    match("LEFT_PAREN");
    if (strcmp(peek().type, "RIGHT_PAREN") != 0) {
        logTransition("parseFunctionCall", peek().value, "parseExpression");
        parseExpression();
        if (TRACK1) printf("parseExpression: DONE\n");
        while (strcmp(peek().type, "COMMA") == 0) {
            match("COMMA");
            logTransition("parseFunctionCall", peek().value, "parseExpression");
            parseExpression();
            if (TRACK1) printf("parseExpression: DONE\n");
        }
    }
    match("RIGHT_PAREN");
}

void parseFunctionDef() {
    logTransition("parseFunctionDef", peek().value, "dispatching");
    match("RESERVED_WORD");
    if (strcmp(peek().type, "IDENTIFIER") != 0) {
        printf("Syntax Error at Line %d: Expected function name.\n", lines);
        errorCount++;
        logTransition("parseFunctionDef", peek().value, "errorRecovery");
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
                logTransition("parseFunctionDef", peek().value, "errorRecovery");
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
                logTransition("parseFunctionDef", peek().value, "errorRecovery");
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

    logTransition("parseFunctionDef", peek().value, "parseBlock");
    parseBlock();
}

void parseExpression() {
    logTransition("parseExpression", peek().value, "parseLogicalOr");
    parseLogicalOr();
    if (TRACK1) printf("parseLogicalOr: DONE\n");
}

void parseLogicalOr() {
    logTransition("parseLogicalOr", peek().value, "parseLogicalAnd");
    parseLogicalAnd();
    if (TRACK1) printf("parseLogicalAnd: DONE\n");
    while (strcmp(peek().type, "LOGICAL") == 0 && strcmp(peek().value, "or") == 0) {
        match("LOGICAL");
        logTransition("parseLogicalOr", peek().value, "parseLogicalAnd");
        parseLogicalAnd();
        if (TRACK1) printf("parseLogicalAnd: DONE\n");
    }
}

void parseLogicalAnd() {
    logTransition("parseLogicalAnd", peek().value, "parseLogicalNot");
    parseLogicalNot();
    if (TRACK1) printf("parseLogicalNot: DONE\n");
    while (strcmp(peek().type, "LOGICAL") == 0 && strcmp(peek().value, "and") == 0) {
        match("LOGICAL");
        logTransition("parseLogicalAnd", peek().value, "parseLogicalNot");
        parseLogicalNot();
        if (TRACK1) printf("parseLogicalNot: DONE\n");
    }
}

void parseLogicalNot() {
    logTransition("parseLogicalNot", peek().value, "dispatching");
    if (strcmp(peek().type, "LOGICAL") == 0 && strcmp(peek().value, "not") == 0) {
        match("LOGICAL");
        logTransition("parseLogicalNot", peek().value, "parseLogicalNot");
        parseLogicalNot();
        if (TRACK1) printf("parseLogicalNot: DONE\n");
        return;
    }
    logTransition("parseLogicalNot", peek().value, "parseRelational");
    parseRelational();
    if (TRACK1) printf("parseRelational: DONE\n");
}

void parseRelational() {

    logTransition("parseRelational", peek().value, "parseArithmetic");
    parseArithmetic();
    while (strcmp(peek().type, "REL_OP") == 0) {
        match("REL_OP");
        logTransition("parseRelational", peek().value, "parseArithmetic");
        parseArithmetic();
        if (TRACK1) printf("parseArithmetic: DONE\n");
    }
}

void parseArithmetic() {
    logTransition("parseArithmetic", peek().value, "parseTerm");
    parseTerm();
    if (TRACK1) printf("parseTerm: DONE\n");
    while (strcmp(peek().type, "ARITH_OP") == 0 &&
          (strcmp(peek().value, "+") == 0 || strcmp(peek().value, "-") == 0)) {
        match("ARITH_OP");
        logTransition("parseArithmetic", peek().value, "parseTerm");
        parseTerm();
        if (TRACK1) printf("parseTerm: DONE\n");
    }
}

void parseTerm() {
    logTransition("parseTerm", peek().value, "parseFactor");
    parseFactor();
    if (TRACK1) printf("parseFactor: DONE\n");
    while (strcmp(peek().type, "ARITH_OP") == 0 &&
          (strcmp(peek().value, "*") == 0 ||
           strcmp(peek().value, "/") == 0 ||
           strcmp(peek().value, "%") == 0 ||
           strcmp(peek().value, "//") == 0)) {
        match("ARITH_OP");
        logTransition("parseTerm", peek().value, "parseFactor");
        parseFactor();
        if (TRACK1) printf("parseFactor: DONE\n");
    }
}

void parseFactor() {
    logTransition("parseFactor", peek().value, "dispatching");
    if (strcmp(peek().type, "ARITH_OP") == 0 &&
        (strcmp(peek().value, "+") == 0 || strcmp(peek().value, "-") == 0)) {
        match("ARITH_OP");
        logTransition("parseFactor", peek().value, "parseFactor");
        parseFactor();
        if (TRACK1) printf("parseFactor: DONE\n");
        return;
    }
    if (strcmp(peek().type, "INCREMENT") == 0 || strcmp(peek().type, "DECREMENT") == 0) {
        match(peek().type);
        match("IDENTIFIER");
        return;
    }
    if (strcmp(peek().type, "NUMBER") == 0 ||
        strcmp(peek().type, "STRING") == 0 ||
        strcmp(peek().type, "CHAR") == 0 ||
        strcmp(peek().type, "STRING_INTERP") == 0) {
        advance();
    } else if (strcmp(peek().type, "IDENTIFIER") == 0) {

        if (strcmp(peekNext().type, "LEFT_PAREN") == 0) {
            logTransition("parseFactor", peek().value, "parseFunctionCall");
            parseFunctionCall();
            if (TRACK1) printf("parseFunctionCall: DONE\n");
        } else {
            if (!isIdentifierDeclared(peek().value)) {
                printf("Semantic Error at Line %d: Identifier '%s' is not declared\n", lines, peek().value);
                errorCount++;
            }
            match("IDENTIFIER");
        }
    } else if (strcmp(peek().type, "LEFT_PAREN") == 0) {
        match("LEFT_PAREN");
        logTransition("parseFactor", peek().value, "parseExpression");
        parseExpression();
        if (TRACK1) printf("parseExpression: DONE\n");
        if (strcmp(peek().type, "RIGHT_PAREN") != 0) {
            printf("Syntax Error at Line %d: Missing closing parenthesis\n", lines);
            errorCount++;
            logTransition("parseFactor", peek().value, "errorRecovery");
            recover();
            return;
        }
        match("RIGHT_PAREN");
    } else if (strcmp(peek().type, "RESERVED_WORD") == 0 &&
               (strcmp(peek().value, "True") == 0 || strcmp(peek().value, "False") == 0 || strcmp(peek().value, "null") == 0)) {
        advance();
    } else {
        printf("Syntax Error at Line %d: Missing operand - Expected NUMBER, IDENTIFIER, STRING, or '(' but got %s (%s)\n", lines,
               peek().type, peek().value);
        errorCount++;
        logTransition("parseFactor", peek().value, "errorRecovery");
        recover();
        return;
    }
    if (strcmp(peek().type, "ARITH_OP") == 0 && strcmp(peek().value, "**") == 0) {
        match("ARITH_OP");
        logTransition("parseFactor", peek().value, "parseFactor");
        parseFactor();
        if (TRACK1) printf("parseFactor: DONE\n");
    }
}
FILE *tokenFile;


Token getNextToken() {
    char line[256];
    Token t = { "EOF", "" };

    // Keep reading lines until we find a REAL token or hit EOF
    while (fgets(line, sizeof(line), tokenFile)) {

        // Basic cleanup
        if (strlen(line) < 3) continue;
        line[strcspn(line, "\n")] = '\0';

        // Parse line format: TYPE(VALUE)
        char *open = strchr(line, '(');
        char *close = strrchr(line, ')');

        if (!open || !close || close < open) continue;

        // Extract Type
        int typeLen = open - line;
        strncpy(t.type, line, typeLen);
        t.type[typeLen] = '\0';

        // 1. Handle Comments (Skip)
        if (strcmp(t.type, "COMMENT") == 0 ||
            strcmp(t.type, "COMMENT_START") == 0 ||
            strcmp(t.type, "COMMENT_END") == 0 ||
            strcmp(t.type, "COM_STR") == 0) {
            continue;
            }

        // 2. Handle Newlines (Increment Line Count, then SKIP)
        if (strcmp(t.type, "NEW_LINE") == 0) {
            lines++;
            continue; // <--- CRITICAL FIX: Go back to start of loop
        }

        // Extract Value
        int valLen = close - open - 1;
        if (valLen > 0) {
            strncpy(t.value, open + 1, valLen);
            t.value[valLen] = '\0';
        } else {
            t.value[0] = '\0';
        }

        // Return the valid token found
        return t;
    }

    return EOF_TOKEN;
}
int main() {
    tokenFile = fopen("Symbol_Table.txt", "r");
    if (!tokenFile) {
        printf("Error: Cannot open Symbol_Table.txt\n");
        return 1;
    }

    lines = 1;
    errorCount = 0;
    braceBalance = 0;

    initLookahead();
    parseProgram();

    fclose(tokenFile);

    if (errorCount > 0) {
        printf("Parsing finished. Total errors: %d\n", errorCount);
    } else {
        printf("Parsing finished successfully.\n");
    }

    return 0;
}