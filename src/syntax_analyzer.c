#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_IDENTIFIERS 1000
#define TRACK 1
#define TRACK1 1
#define MAX_PARAMS 20

typedef struct {
    char type[50];
    char value[100];
    int line;
} Token;

typedef struct {
    char name[100];
    char type[20];
    int isDeclared;
    char value[256];
    int isConst;
    int paramCount;
    char paramTypes[MAX_PARAMS][20];
} Identifier;

Token peekAt(int offset);
Token getNextToken();
void advance();
int isAtEnd();
void recover();
FILE *tokenFile;
FILE *out;


Token lookahead[3];
void initLookahead() {
    for (int i = 0; i < 3; i++) {
        lookahead[i] = getNextToken();
    }
}
int lines = 1;
int errorCount = 0;
int braceBalance = 0;
int loopLevel = 0;
int previousLine = 1;

Identifier symbolTable[MAX_IDENTIFIERS];
int symbolCount = 0;

char currentFunctionReturnType[20] = "none";

Identifier* getIdentifier(const char* name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            return &symbolTable[i];
        }
    }
    return NULL;
}
static Token EOF_TOKEN = { "EOF", "" };

int isAtEnd() {
    return strcmp(peekAt(0).type, "EOF") == 0;
}

Token peekAt(int offset) {
    if (offset >= 0 && offset < 3)
        return lookahead[offset];
    return EOF_TOKEN;
}

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
    if (TRACK) {
        printf("%s --%s--> %s\n", from, input, to);
        fprintf(out, "%s --%s--> %s\n", from, input, to);
    }
}

void advance() {
    if (lookahead[0].line > 0) {
        previousLine = lookahead[0].line;
    }
    lookahead[0] = lookahead[1];
    lookahead[1] = lookahead[2];
    lookahead[2] = getNextToken();

}

int match(const char* expected) {
    if (strcmp(peekAt(0).type, expected) == 0) {
        if (strcmp(expected, "LEFT_BRACE") == 0) braceBalance++;
        if (strcmp(expected, "RIGHT_BRACE") == 0) {
            braceBalance--;
            if (braceBalance < 0) {
                printf("Syntax Error at Line %d: Extra closing brace '}'.\n", peekAt(0).line);
                fprintf(out, "Syntax Error at Line %d: Extra closing brace '}'.\n", peekAt(0).line);
                errorCount++;
                braceBalance = 0;
            }
        }

        advance();
        return 1;
    }
    int reportedLine = peekAt(0).line;

    if (peekAt(0).line > previousLine) {
        reportedLine = previousLine;
    }

    printf("Syntax Error at Line %d: Expected %s but got %s (%s)\n", peekAt(0).line, expected, peekAt(0).type, peekAt(0).value);
    fprintf(out, "Syntax Error at Line %d: Expected %s but got %s (%s)\n", peekAt(0).line, expected, peekAt(0).type, peekAt(0).value);
    errorCount++;
    recover();
    return 0;
}



void declareIdentifier(const char* name, const char* type, int isConst) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].name, name) == 0) {
            printf("Semantic Error at Line %d: Identifier '%s' is already declared.\n", peekAt(0).line, name);
            fprintf(out, "Semantic Error at Line %d: Identifier '%s' is already declared.\n", peekAt(0).line, name);
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
        symbolTable[symbolCount].isConst = isConst;
        symbolTable[symbolCount].paramCount = 0;
        symbolCount++;
    }


    if (symbolCount < MAX_IDENTIFIERS) {
        strncpy(symbolTable[symbolCount].name, name, sizeof(symbolTable[symbolCount].name)-1);
        symbolTable[symbolCount].name[sizeof(symbolTable[symbolCount].name)-1] = '\0';

        strncpy(symbolTable[symbolCount].type, type, sizeof(symbolTable[symbolCount].type)-1);
        symbolTable[symbolCount].type[sizeof(symbolTable[symbolCount].type)-1] = '\0';

        symbolTable[symbolCount].isDeclared = 1;
        symbolTable[symbolCount].isConst = isConst;
        symbolTable[symbolCount].paramCount = 0;
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
                        printf("Semantic Error at Line %d: Identifier '%s' inside string interpolation is not declared.\n", peekAt(0).line, buffer);
                        fprintf(out, "Semantic Error at Line %d: Identifier '%s' inside string interpolation is not declared.\n", peekAt(0).line, buffer);
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
    Token t = peekAt(0);
    return strcmp(t.type, "KEYWORD") == 0 && strcmp(t.value, k) == 0;
}

int isNoise(const char* w) {
    return strcmp(peekAt(0).type, "NOISE_WORD") == 0 &&
           strcmp(peekAt(0).value, w) == 0;
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
    return strcmp(peekAt(0).type, "KEYWORD") == 0 &&
           strcmp(peekAt(0).value, "const") == 0;
}

void recover() {
    logTransition("errorRecovery", peekAt(0).value, "dispatching");
    if (isAtEnd()) return;

    advance();

    while (!isAtEnd()) {

        if (strcmp(peekAt(0).type, "SEMICOLON") == 0) {
            advance();
            return;
        }

        if (strcmp(peekAt(0).type, "RIGHT_BRACE") == 0) {
            return;
        }

        if (strcmp(peekAt(0).type, "RESERVED_WORD") == 0 &&
            isTypeValue(peekAt(0).value)) {
            return;
            }

        if (strcmp(peekAt(0).type, "KEYWORD") == 0) {
            if (strcmp(peekAt(0).value, "if") == 0 ||
                strcmp(peekAt(0).value, "for") == 0 ||
                strcmp(peekAt(0).value, "while") == 0 ||
                strcmp(peekAt(0).value, "return") == 0 ||
                strcmp(peekAt(0).value, "output") == 0 ||
                strcmp(peekAt(0).value, "input") == 0) {
                return;
                }
        }

        if (strcmp(peekAt(0).type, "IDENTIFIER") == 0) {
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
const char* parseExpression();
const char* parseLogicalOr();
const char* parseLogicalAnd();
const char* parseLogicalNot();
const char* parseRelational();
const char* parseArithmetic();
const char* parseTerm();
const char* parseFactor();

void parseProgram() {
    logTransition("parseProgram", peekAt(0).value, "Finding main()");

    if (!isKeyword("main")) {
        printf("Syntax Error: Program must start with main()\n");
        fprintf(out, "Syntax Error: Program must start with main()\n");
        return;
    }
    match("KEYWORD");
    match("LEFT_PAREN");
    match("RIGHT_PAREN");

    logTransition("parseProgram", peekAt(0).value, "parseBlock");
    parseBlock();
    if (TRACK1) {
        printf("parseBlock: DONE\n");
        fprintf(out, "parseBlock: DONE\n");
    }
}

void parseBlock() {
    int hasBegin = 0;
    int hasBrace = 0;
    logTransition("parseBlock", peekAt(0).value, "dispatching");
    if (isNoise("begin")) {
        logTransition("parseBlock","begin", "body");
        match("NOISE_WORD");
        hasBegin = 1;
    }

    if (strcmp(peekAt(0).type, "LEFT_BRACE") == 0) {
        match("LEFT_BRACE");
        hasBrace = 1;
    }

    if (!hasBegin && !hasBrace) {
        printf("Syntax Error at Line %d: Expected '{' or 'begin' to start block but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
        fprintf(out, "Syntax Error at Line %d: Expected '{' or 'begin' to start block but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
        errorCount++;
        recover();
        return;
    }

    while (!isAtEnd()) {
        if (hasBrace && strcmp(peekAt(0).type, "RIGHT_BRACE") == 0) {
            break;
        }
        if (hasBegin && isNoise("end")) {
            break;
        }
        logTransition("parseBlock", peekAt(0).value, "parseStatement");
        parseStatement();
        if (TRACK1) {
            printf("parseStatement: DONE\n");
            fprintf(out, "parseStatement: DONE\n");
        }
    }

    if (hasBrace) {
        match("RIGHT_BRACE");
    }

    if (hasBegin && isNoise("end")) {
        match("NOISE_WORD");

        if (strcmp(peekAt(0).type, "SEMICOLON") == 0) {
            match("SEMICOLON");
        }
    }
}

void parseStatement() {
    logTransition("parseStatement", peekAt(0).value, "If | While | For | Expression | Output | Input | Function Definition | Block | Declaration | Function call | Array assignment");
    if (isKeyword("if")) {
        logTransition("parseStatement", peekAt(0).value, "parseIf");
        parseIf();
        if (TRACK1) {
            printf("parseIf: DONE\n");
            fprintf(out, "parseIf: DONE\n");
        }
        return;
    }
    if (isKeyword("while")) {
        logTransition("parseStatement", peekAt(0).value, "parseWhile");
        parseWhile();
        if (TRACK1) {
            printf("parseWhile: DONE\n");
            fprintf(out, "parseWhile: DONE\n");
        }
        return;
    }

    if (isKeyword("for")) {
        logTransition("parseStatement", peekAt(0).value, "parseFor");
        parseFor();
        if (TRACK1) {
            printf("parseFor: DONE\n");
            fprintf(out, "parseFor: DONE\n");
        }
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
        int line = peekAt(0).line;
        match("KEYWORD");

        if (strcmp(currentFunctionReturnType, "void") == 0) {
            if (strcmp(peekAt(0).type, "SEMICOLON") != 0) {
                printf("Semantic Error at Line %d: Void function cannot return a value.\n", line);
                fprintf(out, "Semantic Error at Line %d: Void function cannot return a value.\n", line);
                errorCount++;
                parseExpression(); 
            }
        } 
        else if (strcmp(currentFunctionReturnType, "none") != 0) { 
             if (strcmp(peekAt(0).type, "SEMICOLON") == 0) {
                 printf("Semantic Error at Line %d: Non-void function '%s' must return a value.\n", line, currentFunctionReturnType);
                 fprintf(out, "Semantic Error at Line %d: Non-void function '%s' must return a value.\n", line, currentFunctionReturnType);
                 errorCount++;
             } else {
                 const char* exprType = parseExpression();
                 if (strcmp(exprType, currentFunctionReturnType) != 0 && strcmp(exprType, "unknown") != 0) {
                     if (!(strcmp(currentFunctionReturnType, "float") == 0 && strcmp(exprType, "int") == 0)) {
                         printf("Semantic Error at Line %d: Return type mismatch. Expected %s, got %s.\n", line, currentFunctionReturnType, exprType);
                         fprintf(out, "Semantic Error at Line %d: Return type mismatch. Expected %s, got %s.\n", line, currentFunctionReturnType, exprType);
                         errorCount++;
                     }
                 }
             }
        }
        else {
             if (strcmp(peekAt(0).type, "SEMICOLON") != 0) parseExpression();
        }
        match("SEMICOLON");
        return;
    }

    if (isKeyword("output")) {
        logTransition("parseStatement", peekAt(0).value, "parseOutput");
        parseOutput();
        if (TRACK1) {
            printf("parseOutput: DONE\n");
            fprintf(out, "parseOutput: DONE\n");
        }
        return;
    }

    if (strcmp(peekAt(0).type, "IDENTIFIER") == 0 &&
        strcmp(peekAt(1).type, "ASSIGN_OP") == 0 &&
        isKeywordAt(2, "input")) {
        logTransition("parseStatement", peekAt(0).value, "parseInput");
        parseInputStatement();
        if (TRACK1) {
            printf("parseInput: DONE\n");
            fprintf(out, "parseInput: DONE\n");
        }
        return;
    }

    if (strcmp(peekAt(0).type, "RESERVED_WORD") == 0 &&
        isTypeValue(peekAt(0).value) &&
        strcmp(peekAt(1).type, "IDENTIFIER") == 0 &&
        strcmp(peekAt(2).type, "LEFT_PAREN") == 0) {
        logTransition("parseStatement", peekAt(0).value, "parseFunctionDef");
        parseFunctionDef();
        if (TRACK1) {
            printf("parseFunctionDef: DONE\n");
            fprintf(out, "parseFunctionDef: DONE\n");
        }
        return;
    }

    if (isNoise("begin")) {
        match("NOISE_WORD");
        logTransition("parseStatement", peekAt(0).value, "parseBlock");
        parseBlock();
        if (TRACK1) {
            printf("parseBlock: DONE\n");
            fprintf(out, "parseBlock: DONE\n");
        }
        return;
    }
    if (strcmp(peekAt(0).type, "LEFT_BRACE") == 0) {
        logTransition("parseStatement", peekAt(0).value, "parseBlock");
        parseBlock();
        if (TRACK1) {
            printf("parseBlock: DONE\n");
            fprintf(out, "parseBlock: DONE\n");
        }
        return;
    }

    if (isConstTypeStart()) {
        logTransition("parseStatement", peekAt(0).value, "parseDeclaration");
        parseDeclaration();
        if (TRACK1) {
            printf("parseDeclaration: DONE\n");
            fprintf(out, "parseDeclaration: DONE\n");
        }
        return;
    }

    if (strcmp(peekAt(0).type, "RESERVED_WORD") == 0 && isTypeValue(peekAt(0).value)) {
        logTransition("parseStatement", peekAt(0).value, "parseDeclaration");
        parseDeclaration();
        if (TRACK1) {
            printf("parseDeclaration: DONE\n");
            fprintf(out, "parseDeclaration: DONE\n");
        }
        return;
    }

    if (strcmp(peekAt(0).type, "IDENTIFIER") == 0 && strcmp(peekAt(1).type, "LEFT_PAREN") == 0) {
        logTransition("parseStatement", peekAt(0).value, "parseFunctionCall");
        parseFunctionCall();
        if (TRACK1) {
            printf("parseFunctionCall: DONE\n");
            fprintf(out, "parseFunctionCall: DONE\n");
        }

        match("SEMICOLON");
        return;
    }

    if (strcmp(peekAt(0).type, "IDENTIFIER") == 0 && strcmp(peekAt(1).type, "LEFT_BRACKET") == 0) {
        logTransition("parseStatement", peekAt(0).value, "parseArrayAssignment");
        parseArrayAssignment();
        if (TRACK1) {
            printf("parseArrayAssignment: DONE\n");
            fprintf(out, "parseArrayAssignment: DONE\n");
        }
        match("SEMICOLON");
        return;
    }

    if (strcmp(peekAt(0).type, "INCREMENT") == 0 || strcmp(peekAt(0).type, "DECREMENT") == 0) {
        advance();
        logTransition("parseStatement", "IDENTIFIER", "end");
        match("IDENTIFIER");
        match("SEMICOLON");
        return;
    }

    if (strcmp(peekAt(0).type, "IDENTIFIER") == 0 &&
       (strcmp(peekAt(1).type, "INCREMENT") == 0 || strcmp(peekAt(1).type, "DECREMENT") == 0)) {
        logTransition("parseStatement", "IDENTIFIER", "end");
        match("IDENTIFIER");
        advance();
        match("SEMICOLON");
        return;
    }
    if (strcmp(peekAt(0).type, "IDENTIFIER") == 0 && strcmp(peekAt(1).type, "ASSIGN_OP") == 0) {
        logTransition("parseStatement", peekAt(0).value, "parseAssignment");
        if (TRACK1) {
            printf("parseAssignment: DONE\n");
            fprintf(out, "parseAssignment: DONE\n");
        }
        parseAssignment();
        return;
    }
    if (strcmp(peekAt(0).type, "NEW_LINE") == 0) {
        advance();
    }

    printf("Syntax Error at Line %d: Unknown statement starting with %s (%s)\n",peekAt(0).line, peekAt(0).type, peekAt(0).value);
    fprintf(out, "Syntax Error at Line %d: Unknown statement starting with %s (%s)\n",peekAt(0).line, peekAt(0).type, peekAt(0).value);
    errorCount++;
    logTransition("parseStatement", peekAt(0).value, "errorRecovery");
    recover();
}

void parseDeclaration() {
    int isConst = 0;

    if (isConstTypeStart()) {
        isConst = 1;
        match("KEYWORD");
    }

    char declaredType[20];
    strncpy(declaredType, peekAt(0).value, sizeof(declaredType)-1);
    declaredType[sizeof(declaredType)-1] = '\0';

    match("RESERVED_WORD");

    if (strcmp(peekAt(0).type, "IDENTIFIER") != 0) {
        printf("Syntax Error at Line %d: Expected identifier in declaration.\n", peekAt(0).line);
        fprintf(out, "Syntax Error at Line %d: Expected identifier in declaration.\n", peekAt(0).line);
        errorCount++;
        recover();
        return;
    }

    if (strcmp(peekAt(1).type, "LEFT_BRACKET") == 0) {
        logTransition("parseDeclaration", peekAt(0).value, "parseArrayAssignment");
        char idname[100];
        strncpy(idname, peekAt(0).value, sizeof(idname)-1);
        idname[sizeof(idname)-1] = '\0';

        declareIdentifier(idname, declaredType, isConst);
        parseArrayAssignment();
        if (TRACK1) {
            printf("parseArrayAssignment: DONE\n");
            fprintf(out, "parseArrayAssignment: DONE\n");
        }
        return;
    }

    while (1) {
        char idname[100];
        strncpy(idname, peekAt(0).value, sizeof(idname)-1);
        idname[sizeof(idname)-1] = '\0';
        match("IDENTIFIER");

        declareIdentifier(idname, declaredType, isConst);

        if (strcmp(peekAt(0).type, "ASSIGN_OP") == 0) {
            match("ASSIGN_OP");

            if (strcmp(peekAt(0).type, "RESERVED_WORD") == 0 && isTypeValue(peekAt(0).value) &&
                strcmp(peekAt(1).type, "LEFT_PAREN") == 0 && isKeywordAt(2, "input")) {

                char castType[20];
                strncpy(castType, peekAt(0).value, sizeof(castType)-1);
                castType[sizeof(castType)-1] = '\0';

                if (strcmp(declaredType, castType) != 0) {
                    printf("Semantic Error at Line %d: Type mismatch. Cannot assign %s(input) to variable of type %s.\n", peekAt(0).line, castType, declaredType);
                    fprintf(out, "Semantic Error at Line %d: Type mismatch. Cannot assign %s(input) to variable of type %s.\n", peekAt(0).line, castType, declaredType);
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
                     printf("Semantic Error at Line %d: Type mismatch. 'input()' returns char/str, but variable is type %s.\n", peekAt(0).line, declaredType);
                    fprintf(out, "Semantic Error at Line %d: Type mismatch. 'input()' returns char/str, but variable is type %s.\n", peekAt(0).line, declaredType);
                     errorCount++;
                }

                match("KEYWORD");
                match("LEFT_PAREN");
                match("STRING");
                match("RIGHT_PAREN");
            }
            else {
                logTransition("ASSIGN_OP", peekAt(0).value, "parseExpression");

                // 1. Capture the type of the value being assigned
                const char* rhsType = parseExpression();

                if (TRACK1) {
                    printf("parseExpression: DONE\n");
                    fprintf(out, "parseExpression: DONE\n");
                }

                // 2. Compare declared type vs value type
                if (strcmp(declaredType, rhsType) != 0) {
                    // (Optional) Allow assigning int to float?
                    if (strcmp(declaredType, "float") == 0 && strcmp(rhsType, "int") == 0) {
                        // Safe implicit cast, do nothing
                    } else {
                        printf("Semantic Error at Line %d: Type mismatch. Cannot assign %s to variable of type %s.\n", peekAt(0).line, rhsType, declaredType);
                        fprintf(out, "Semantic Error at Line %d: Type mismatch. Cannot assign %s to variable of type %s.\n", peekAt(0).line, rhsType, declaredType);
                        errorCount++;
                    }
                }
            }
        }

        if (strcmp(peekAt(0).type, "COMMA") == 0) {
            match("COMMA");
            logTransition("COMMA", peekAt(0).value, "parseDeclaration");
            continue;
        } else break;
    }

    match("SEMICOLON");
}

int expectCondition() {
    if (strcmp(peekAt(0).type, "LEFT_PAREN") != 0) {
        printf("Syntax Error at Line %d: Expected '(' to start condition but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
        fprintf(out, "Syntax Error at Line %d: Expected '(' to start condition but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
        errorCount++;
        recover();
        return 0;
    }
    match("LEFT_PAREN");

    if (isAtEnd() || strcmp(peekAt(0).type, "RIGHT_PAREN") == 0) {
        printf("Syntax Error at Line %d: Expected condition expression inside parentheses.\n", peekAt(0).line);
        fprintf(out, "Syntax Error at Line %d: Expected condition expression inside parentheses.\n", peekAt(0).line);
        errorCount++;
        recover();
        return 0;
    }
    const char* condType = parseExpression();

    if (strcmp(condType, "bool") != 0 && strcmp(condType, "unknown") != 0) {
        printf("Semantic Error at Line %d: Condition must be boolean, got %s.\n",
               peekAt(0).line, condType);
        fprintf(out, "Semantic Error at Line %d: Condition must be boolean, got %s.\n",
               peekAt(0).line, condType);
        errorCount++;
    }
    if (strcmp(peekAt(0).type, "RIGHT_PAREN") != 0) {
        printf("Syntax Error at Line %d: Expected ')' to close condition but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
        fprintf(out, "Syntax Error at Line %d: Expected ')' to close condition but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
        errorCount++;
        recover();
        return 0;
    }
    match("RIGHT_PAREN");
    return 1;
}

void parseIf() {
    logTransition("parseIf", "if", "condition");
    if (strcmp(peekAt(0).type, "KEYWORD") != 0 || strcmp(peekAt(0).value, "if") != 0) {
        printf("Internal Error: parseIf called but current token is %s (%s)\n", peekAt(0).type, peekAt(0).value);
        fprintf(out, "Internal Error: parseIf called but current token is %s (%s)\n", peekAt(0).type, peekAt(0).value);
        errorCount++;
        logTransition("parseIf", peekAt(0).value, "errorRecovery");
        recover();
        return;
    }
    match("KEYWORD");
    logTransition("parseIf", "condition", "parseExpression");
    if (!expectCondition()) {
        if (strcmp(peekAt(0).type, "LEFT_BRACE") == 0 || isNoise("begin")) {
            logTransition("parseIf", peekAt(0).value, "parseBlock");
            parseBlock();
            if (TRACK1) {
                printf("parseBlock: DONE\n");
                fprintf(out, "parseBlock: DONE\n");
            }
        } else {
            logTransition("parseIf", peekAt(0).value, "parseStatement");
            parseStatement();
            if (TRACK1) {
                printf("parseStatement: DONE\n");
                fprintf(out, "parseStatement: DONE\n");
            }
        }
        return;
    }

    if (isNoise("then")) match("NOISE_WORD");
    logTransition("condition", peekAt(0).value, "Block | Statement");
    if (strcmp(peekAt(0).type, "LEFT_BRACE") == 0 || isNoise("begin")) {
        logTransition("parseIf", peekAt(0).value, "parseBlock");
        parseBlock();
        if (TRACK1) {
            printf("parseBlock: DONE\n");
            fprintf(out, "parseBlock: DONE\n");
        }
    } else {
        logTransition("parseIf", peekAt(0).value, "parseStatement");
        parseStatement();
        if (TRACK1) {
            printf("parseStatement: DONE\n");
            fprintf(out, "parseStatement: DONE\n");
        }
    }

    while (isKeyword("else")) {
        match("KEYWORD");

        if (isKeyword("if")) {
            match("KEYWORD");
            logTransition("parseIf", "else if condition", "parseExpression");
            if (!expectCondition()) {
                if (strcmp(peekAt(0).type, "LEFT_BRACE") == 0 || isNoise("begin")) {
                    logTransition("parseIf", "condition", "parseBlock");
                    parseBlock();
                    if (TRACK1) {
                        printf("parseBlock: DONE\n");
                        fprintf(out, "parseBlock: DONE\n");
                    }
                } else {
                    logTransition("parseIf", "condition", "parseStatement");
                    parseStatement();
                    if (TRACK1) {
                        printf("parseStatement: DONE\n");
                        fprintf(out, "parseStatement: DONE\n");
                    }
                }
                break;
            }
            if (isNoise("then")) match("NOISE_WORD");

            if (strcmp(peekAt(0).type, "LEFT_BRACE") == 0 || isNoise("begin")) {
                logTransition("parseIf", peekAt(0).value, "parseBlock");
                parseBlock();
                if (TRACK1) {
                    printf("parseBlock: DONE\n");
                    fprintf(out, "parseBlock: DONE\n");
                }
            } else {
                logTransition("parseIf", peekAt(0).value, "parseStatement");
                parseStatement();
                if (TRACK1) {
                    printf("parseStatement: DONE\n");
                    fprintf(out, "parseStatement: DONE\n");
                }
            }
        } else {
            logTransition("parseIf", "else", "Block | Statement");
            if (isNoise("then")) match("NOISE_WORD");
            if (strcmp(peekAt(0).type, "LEFT_BRACE") == 0 || isNoise("begin")) {
                logTransition("parseIf", peekAt(0).value, "parseBlock");
                parseBlock();
                if (TRACK1) {
                    printf("parseBlock: DONE\n");
                    fprintf(out, "parseBlock: DONE\n");
                }
            } else {
                logTransition("parseIf", peekAt(0).value, "parseStatement");
                parseStatement();
                if (TRACK1) {
                    printf("parseStatement: DONE\n");
                    fprintf(out, "parseStatement: DONE\n");
                }
            }
            break;
        }
    }
}

void parseWhile() {
    if (strcmp(peekAt(0).type, "KEYWORD") != 0 || strcmp(peekAt(0).value, "while") != 0) {
        printf("Internal Error: parseWhile called but current token is %s (%s)\n", peekAt(0).type, peekAt(0).value);
        fprintf(out, "Internal Error: parseWhile called but current token is %s (%s)\n", peekAt(0).type, peekAt(0).value);
        errorCount++;
        logTransition("parseWhile", peekAt(0).value, "errorRecovery");
        recover();
        return;
    }
    match("KEYWORD");
    logTransition("parseWhile", "condition", "parseExpression");
    if (!expectCondition()) {
        if (isNoise("do")) match("NOISE_WORD");
        if (strcmp(peekAt(0).type, "LEFT_BRACE") == 0 || isNoise("begin")) {
            logTransition("parseWhile", peekAt(0).value, "parseBlock");
            parseBlock();
            if (TRACK1) {
                printf("parseBlock: DONE\n");
                fprintf(out, "parseBlock: DONE\n");
            }
        } else {
            logTransition("parseWhile", peekAt(0).value, "parseStatement");
            parseStatement();
            if (TRACK1) {
                printf("parseStatement: DONE\n");
                fprintf(out, "parseStatement: DONE\n");
            }
        }
        return;
    }
    logTransition("parseWhile", "condition", "body");
    if (strcmp(peekAt(0).type, "LEFT_BRACE") == 0 || isNoise("begin") || isNoise("do")) {
        if (isNoise("do")) match("NOISE_WORD");
        logTransition("parseWhile", peekAt(0).value, "parseBlock");
        parseBlock();
        if (TRACK1) {
            printf("parseBlock: DONE\n");
            fprintf(out, "parseBlock: DONE\n");
        }
    } else {
        logTransition("parseWhile", peekAt(0).value, "parseStatement");
        parseStatement();
        if (TRACK1) {
            printf("parseStatement: DONE\n");
            fprintf(out, "parseStatement: DONE\n");
        }
    }
}

void parseFor() {
    logTransition("parseFor", peekAt(0).value, "dispatching");
    if (strcmp(peekAt(0).type, "KEYWORD") != 0 || strcmp(peekAt(0).value, "for") != 0) {
        printf("Internal Error: parseFor called but current token is %s (%s)\n", peekAt(0).type, peekAt(0).value);
        fprintf(out, "Internal Error: parseFor called but current token is %s (%s)\n", peekAt(0).type, peekAt(0).value);
        errorCount++;
        logTransition("parseFor", peekAt(0).value, "errorRecovery");
        recover();
        return;
    }
    match("KEYWORD");

    if (strcmp(peekAt(0).type, "LEFT_PAREN") != 0) {
        printf("Syntax Error at Line %d: Expected '(' after for but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
        fprintf(out, "Syntax Error at Line %d: Expected '(' after for but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
        errorCount++;
        logTransition("parseFor", peekAt(0).value, "errorRecovery");
        recover();
    } else {
        match("LEFT_PAREN");
        logTransition("parseFor", "condition", "Semicolon | Declaration | Expression | Assignment");
    }

    int initConsumedSemicolon = 0;
    if (strcmp(peekAt(0).type, "SEMICOLON") == 0) {
        match("SEMICOLON");
        initConsumedSemicolon = 1;
    } else if (strcmp(peekAt(0).type, "RESERVED_WORD") == 0 && isTypeValue(peekAt(0).value)) {
        logTransition("parseFor", peekAt(0).value, "parseDeclaration");
        parseDeclaration();
        if (TRACK1) {
            printf("parseDeclaration: DONE\n");
            fprintf(out, "parseDeclaration: DONE\n");
        }
        initConsumedSemicolon = 1;
    } else {
        if (strcmp(peekAt(0).type, "IDENTIFIER") == 0 && strcmp(peekAt(1).type, "ASSIGN_OP") == 0) {
            logTransition("parseFor", peekAt(0).value, "parseAssignment");
            parseAssignment();
            if (TRACK1) {
                printf("parseAssigment: DONE\n");
                fprintf(out, "parseAssigment: DONE\n");
            }
            initConsumedSemicolon = 1;
        } else {
            logTransition("parseFor", "condition", "parseExpression");
            parseExpression();
            if (TRACK1) {
                printf("parseExpression: DONE\n");
                fprintf(out, "parseExpression: DONE\n");
            }
            if (strcmp(peekAt(0).type, "SEMICOLON") == 0) {
                match("SEMICOLON");
                initConsumedSemicolon = 1;
            } else {
                printf("Syntax Error at Line %d: Expected ';' after for-initialization but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
                fprintf(out, "Syntax Error at Line %d: Expected ';' after for-initialization but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
                errorCount++;
                logTransition("parseFor", peekAt(0).value, "errorRecovery");
                recover();
            }
        }
    }

    if (!initConsumedSemicolon) {
        if (strcmp(peekAt(0).type, "SEMICOLON") == 0) {
            match("SEMICOLON");
        } else {
            printf("Syntax Error at Line %d: Expected ';' after for-initialization but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
            fprintf(out, "Syntax Error at Line %d: Expected ';' after for-initialization but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
            errorCount++;
            logTransition("parseFor", peekAt(0).value, "errorRecovery");
            recover();
        }
    }

    if (strcmp(peekAt(0).type, "SEMICOLON") == 0) {
        match("SEMICOLON");
    } else {
        logTransition("parseFor", peekAt(0).value, "parseExpression");
        parseExpression();
        if (strcmp(peekAt(0).type, "SEMICOLON") == 0) {
            match("SEMICOLON");
        } else {
            printf("Syntax Error at Line %d: Expected ';' after for-condition but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
            fprintf(out, "Syntax Error at Line %d: Expected ';' after for-condition but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
            errorCount++;
            logTransition("parseFor", peekAt(0).value, "errorRecovery");
            recover();
        }
    }


    if (strcmp(peekAt(0).type, "RIGHT_PAREN") == 0) {
        logTransition("parseFor", "condition", "body");
    } else {
        if (strcmp(peekAt(0).type, "IDENTIFIER") == 0 && strcmp(peekAt(1).type, "ASSIGN_OP") == 0) {
            char namebuf[100];
            strncpy(namebuf, peekAt(0).value, sizeof(namebuf)-1);
            namebuf[sizeof(namebuf)-1] = '\0';
            match("IDENTIFIER");
            match("ASSIGN_OP");
            logTransition("parseFor", peekAt(0).value, "parseExpression");
            parseExpression();
            if (TRACK1) {
                printf("parseExpression: DONE\n");
                fprintf(out, "parseExpression: DONE\n");
            }
            if (!isIdentifierDeclared(namebuf)) {
                errorCount++;
                printf("Semantic Error at Line %d: Identifier '%s' assigned but not declared.\n", peekAt(0).line, namebuf);
                fprintf(out, "Semantic Error at Line %d: Identifier '%s' assigned but not declared.\n", peekAt(0).line, namebuf);
            }
        } else if (strcmp(peekAt(0).type, "IDENTIFIER") == 0 && (strcmp(peekAt(1).type, "INCREMENT") == 0 || strcmp(peekAt(1).type, "DECREMENT") == 0)) {
            match("IDENTIFIER");
            advance();
        } else if (strcmp(peekAt(0).type, "INCREMENT") == 0 || strcmp(peekAt(0).type, "DECREMENT") == 0) {
            advance();
            match("IDENTIFIER");
        } else {
            logTransition("parseFor", peekAt(0).value, "parseExpression");
            parseExpression();
            if (TRACK1) {
                printf("parseExpression: DONE\n");
                fprintf(out, "parseExpression: DONE\n");
            }
        }
    }

    if (strcmp(peekAt(0).type, "RIGHT_PAREN") == 0) {
        match("RIGHT_PAREN");
        logTransition("parseFor", "condition", "body");
    } else {
        printf("Syntax Error at Line %d: Expected ')' to close for loop header but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
        fprintf(out, "Syntax Error at Line %d: Expected ')' to close for loop header but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
        errorCount++;
        logTransition("parseFor", peekAt(0).value, "errorRecovery");
        recover();
    }


    if (isNoise("do")) match("NOISE_WORD");

    if (strcmp(peekAt(0).type, "LEFT_BRACE") == 0 || isNoise("begin")) {
        logTransition("parseFor", peekAt(0).value, "parseBlock");
        parseBlock();
        if (TRACK1) {
            printf("parseBlock: DONE\n");
            fprintf(out, "parseBlock: DONE\n");
        }
    } else {
        logTransition("parseFor", peekAt(0).value, "parseStatement");
        parseStatement();
        if (TRACK1) {
            printf("parseStatement: DONE\n");
            fprintf(out, "parseStatement: DONE\n");
        }
    }
}

void parseAssignment() {
    logTransition("parseAssignment", peekAt(0).value, "dispatching");

    if (strcmp(peekAt(0).type, "IDENTIFIER") != 0) {
        printf("Syntax Error at Line %d: Assignment must start with identifier.\n", peekAt(0).line);
        fprintf(out, "Syntax Error at Line %d: Assignment must start with identifier.\n", peekAt(0).line);
        errorCount++;
        recover();
        return;
    }

    char namebuf[100];
    strncpy(namebuf, peekAt(0).value, sizeof(namebuf)-1);
    namebuf[sizeof(namebuf)-1] = '\0';
    Identifier* id = getIdentifier(namebuf);
    if(id && id->isConst){
        printf("Semantic Error at Line %d: Cannot reassign constant '%s'.\n", peekAt(0).line, namebuf);
        fprintf(out, "Semantic Error at Line %d: Cannot reassign constant '%s'.\n", peekAt(0).line, namebuf);
        errorCount++;
    }

    const char* targetType = getIdentifierType(namebuf);

    match("IDENTIFIER");
    match("ASSIGN_OP");

    if (strcmp(peekAt(0).type, "RESERVED_WORD") == 0 && isTypeValue(peekAt(0).value) &&
        strcmp(peekAt(1).type, "LEFT_PAREN") == 0 && isKeywordAt(2, "input")) {

        char castType[20];
        strncpy(castType, peekAt(0).value, sizeof(castType)-1);
        castType[sizeof(castType)-1] = '\0';

        if (strcmp(targetType, "unknown") != 0 && strcmp(targetType, castType) != 0) {
             printf("Semantic Error at Line %d: Type mismatch. Cannot assign %s(input) to variable '%s' of type %s.\n", peekAt(0).line, castType, namebuf, targetType);
            fprintf(out, "Semantic Error at Line %d: Type mismatch. Cannot assign %s(input) to variable '%s' of type %s.\n", peekAt(0).line, castType, namebuf, targetType);
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
    else if (strcmp(peekAt(0).type, "KEYWORD") == 0 && strcmp(peekAt(0).value, "input") == 0) {
        if (strcmp(targetType, "unknown") != 0 && strcmp(targetType, "str") != 0 && strcmp(targetType, "char") != 0) {
             printf("Semantic Error at Line %d: Type mismatch. 'input()' returns char/str, but variable '%s' is type %s.\n", peekAt(0).line, namebuf, targetType);
            fprintf(out, "Semantic Error at Line %d: Type mismatch. 'input()' returns char/str, but variable '%s' is type %s.\n", peekAt(0).line, namebuf, targetType);
             errorCount++;
        }

        match("KEYWORD");
        match("LEFT_PAREN");
        match("STRING");
        match("RIGHT_PAREN");
    } else {
        logTransition("parseAssignment", peekAt(0).value, "parseExpression");

        // 1. Capture the type
        const char* rhsType = parseExpression();

        if (TRACK1) {
            printf("parseExpression: DONE\n");
            fprintf(out, "parseExpression: DONE\n");
        }

        if (strcmp(targetType, "unknown") != 0 && strcmp(rhsType, "unknown") != 0) {
            if (strcmp(targetType, rhsType) != 0) {
                if (strcmp(targetType, "float") == 0 && strcmp(rhsType, "int") == 0) {

                } else {
                    printf("Semantic Error at Line %d: Type mismatch. Cannot assign %s to variable '%s' of type %s.\n", peekAt(0).line, rhsType, namebuf, targetType);
                    fprintf(out, "Semantic Error at Line %d: Type mismatch. Cannot assign %s to variable '%s' of type %s.\n", peekAt(0).line, rhsType, namebuf, targetType);
                    errorCount++;
                }
            }
        }
    }

    match("SEMICOLON");

    if (!isIdentifierDeclared(namebuf)) {
        errorCount++;
        printf("Semantic Error at Line %d: Identifier '%s' assigned but not declared.\n", peekAt(0).line, namebuf);
        fprintf(out, "Semantic Error at Line %d: Identifier '%s' assigned but not declared.\n", peekAt(0).line, namebuf);
    }
}

void parseArrayAssignment() {
    logTransition("parseArrayAssignment", peekAt(0).value, "dispatching");

    if (strcmp(peekAt(0).type, "IDENTIFIER") == 0) match("IDENTIFIER");

    match("LEFT_BRACKET");

    if (strcmp(peekAt(0).type, "RIGHT_BRACKET") != 0) {
        logTransition("parseArrayAssignment", peekAt(0).value, "parseExpression");
        parseExpression();
    }
    match("RIGHT_BRACKET");

    match("ASSIGN_OP");

    if (strcmp(peekAt(0).type, "LEFT_BRACE") == 0) {
        match("LEFT_BRACE");
        if (strcmp(peekAt(0).type, "RIGHT_BRACE") != 0) {
            logTransition("parseArrayAssignment", peekAt(0).value, "parseExpression");
            parseExpression();
            if (TRACK1) {
                printf("parseExpression: DONE\n");
                fprintf(out, "parseExpression: DONE\n");
            }
            while (strcmp(peekAt(0).type, "COMMA") == 0) {
                match("COMMA");
                logTransition("parseArrayAssignment", peekAt(0).value, "parseExpression");
                parseExpression();
                if (TRACK1) {
                    printf("parseExpression: DONE\n");
                    fprintf(out, "parseExpression: DONE\n");
                }
            }
        }
        match("RIGHT_BRACE");
    } else {
        logTransition("parseArrayAssignment", peekAt(0).value, "parseExpression");
        parseExpression();
        if (TRACK1) {
            printf("parseExpression: DONE\n");
            fprintf(out, "parseExpression: DONE\n");
        }
    }
    match("SEMICOLON");
}

void parseInputStatement() {
    logTransition("parseInput", peekAt(0).value, "dispatching");
    if (strcmp(peekAt(0).type, "IDENTIFIER") != 0) {
        printf("Semantic Error at Line %d: input assignment must start with identifier.\n", peekAt(0).line);
        fprintf(out, "Semantic Error at Line %d: input assignment must start with identifier.\n", peekAt(0).line);
        errorCount++;
        recover();
        return;
    }

    char namebuf[100];
    strncpy(namebuf, peekAt(0).value, sizeof(namebuf)-1);
    namebuf[sizeof(namebuf)-1] = '\0';

    if (!isIdentifierDeclared(namebuf)) {
        printf("Semantic Error at Line %d: Identifier '%s' is not declared.\n", peekAt(0).line, namebuf);
        fprintf(out, "Semantic Error at Line %d: Identifier '%s' is not declared.\n", peekAt(0).line, namebuf);
        errorCount++;
    }

    const char* targetType = getIdentifierType(namebuf);

    match("IDENTIFIER");
    match("ASSIGN_OP");
    match("KEYWORD");

    if (strcmp(targetType, "unknown") != 0 && strcmp(targetType, "str") != 0 && strcmp(targetType, "char") != 0) {
        printf("Semantic Error at Line %d: Type mismatch. 'input()' returns char/str, but variable '%s' is type %s.\n", peekAt(0).line, namebuf, targetType);
        fprintf(out, "Semantic Error at Line %d: Type mismatch. 'input()' returns char/str, but variable '%s' is type %s.\n", peekAt(0).line, namebuf, targetType);
        errorCount++;
    }

    match("LEFT_PAREN");
    if (strcmp(peekAt(0).type, "STRING") == 0)
        match("STRING");
    else if (strcmp(peekAt(0).type, "STRING_INTERP") == 0)
        match("STRING_INTERP");
    else {
        printf("Syntax Error at Line %d: input prompt must be a string literal.\n", peekAt(0).line);
        fprintf(out, "Syntax Error at Line %d: input prompt must be a string literal.\n", peekAt(0).line);
        errorCount++;
        recover();
        return;
    }
    match("RIGHT_PAREN");
    match("SEMICOLON");
}

void parseOutput() {
    logTransition("parseOutput", peekAt(0).value, "dispatching");

    match("KEYWORD");
    match("LEFT_PAREN");

    if (strcmp(peekAt(0).type, "STRING") != 0 && strcmp(peekAt(0).type, "STRING_INTERP") != 0) {
        errorCount++;
        printf("Syntax Error at Line %d: Output statement must start with a string literal.\n", peekAt(0).line);
        fprintf(out, "Syntax Error at Line %d: Output statement must start with a string literal.\n", peekAt(0).line);
        logTransition("parseOutput", peekAt(0).value, "errorRecovery");
        recover();
        return;
    }

    validateStringInterpolation(peekAt(0).value);
    if (strcmp(peekAt(0).type, "STRING_INTERP") == 0) {
        char temp[256];
        strcpy(temp, peekAt(0).value);
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

        }

        match("STRING_INTERP");
    }
    else {
        match("STRING");
    }



    if (strcmp(peekAt(0).type, "COMMA") == 0) {
        errorCount++;
        printf("Syntax Error at Line %d: Output statement cannot accept multiple arguments.\n", peekAt(0).line);
        fprintf(out, "Syntax Error at Line %d: Output statement cannot accept multiple arguments.\n", peekAt(0).line);
        logTransition("parseOutput", peekAt(0).value, "errorRecovery");
        recover();
        return;
    }

    match("RIGHT_PAREN");
    match("SEMICOLON");
}

void parseFunctionCall() {
    logTransition("parseFunctionCall", peekAt(0).value, "dispatching");
    
    char funcName[100];
    strncpy(funcName, peekAt(0).value, sizeof(funcName)-1);
    funcName[sizeof(funcName)-1] = '\0';
    int line = peekAt(0).line;

    match("IDENTIFIER");
    match("LEFT_PAREN");

    Identifier* funcId = getIdentifier(funcName);
    int argCount = 0;

    if (!funcId) {
        printf("Semantic Error at Line %d: Function '%s' is not declared.\n", line, funcName);
        fprintf(out, "Semantic Error at Line %d: Function '%s' is not declared.\n", line, funcName);
        errorCount++;
    }

    if (strcmp(peekAt(0).type, "RIGHT_PAREN") != 0) {
    
        const char* argType = parseExpression();
        
        if (funcId && argCount < funcId->paramCount) {
             const char* expected = funcId->paramTypes[argCount];
             if (strcmp(argType, expected) != 0 && strcmp(argType, "unknown") != 0) {
                  if (!(strcmp(expected, "float") == 0 && strcmp(argType, "int") == 0)) {
                       printf("Semantic Error at Line %d: Argument %d expects %s, got %s.\n", line, argCount+1, expected, argType);
                       fprintf(out, "Semantic Error at Line %d: Argument %d expects %s, got %s.\n", line, argCount+1, expected, argType);
                       errorCount++;
                  }
             }
        }
        argCount++;

        while (strcmp(peekAt(0).type, "COMMA") == 0) {
            match("COMMA");
            argType = parseExpression();
            
            if (funcId && argCount < funcId->paramCount) {
                const char* expected = funcId->paramTypes[argCount];
                if (strcmp(argType, expected) != 0 && strcmp(argType, "unknown") != 0) {
                     if (!(strcmp(expected, "float") == 0 && strcmp(argType, "int") == 0)) {
                        printf("Semantic Error at Line %d: Argument %d expects %s, got %s.\n", line, argCount+1, expected, argType);
                        fprintf(out, "Semantic Error at Line %d: Argument %d expects %s, got %s.\n", line, argCount+1, expected, argType);
                        errorCount++;
                     }
                }
            }
            argCount++;
        }
    }
    match("RIGHT_PAREN");

    if (funcId) {
        if (argCount != funcId->paramCount) {
            printf("Semantic Error at Line %d: Function '%s' expects %d arguments, got %d.\n", line, funcName, funcId->paramCount, argCount);
            fprintf(out, "Semantic Error at Line %d: Function '%s' expects %d arguments, got %d.\n", line, funcName, funcId->paramCount, argCount);
            errorCount++;
        }
    }
}

void parseFunctionDef() {
    logTransition("parseFunctionDef", peekAt(0).value, "dispatching");
    
    char returnType[20];
    strncpy(returnType, peekAt(0).value, sizeof(returnType)-1);
    returnType[sizeof(returnType)-1] = '\0';
    match("RESERVED_WORD");

    if (strcmp(peekAt(0).type, "IDENTIFIER") != 0) {
        printf("Syntax Error at Line %d: Expected function name.\n", peekAt(0).line);
        fprintf(out, "Syntax Error at Line %d: Expected function name.\n", peekAt(0).line);
        errorCount++;
        recover();
        return;
    }

    char fname[100];
    strncpy(fname, peekAt(0).value, sizeof(fname)-1);
    fname[sizeof(fname)-1] = '\0';
    
    declareIdentifier(fname, returnType, 0);
    
    strncpy(currentFunctionReturnType, returnType, sizeof(currentFunctionReturnType)-1);

    Identifier* funcId = getIdentifier(fname);
    if (funcId) funcId->paramCount = 0;

    match("IDENTIFIER");
    match("LEFT_PAREN");

    if (strcmp(peekAt(0).type, "RIGHT_PAREN") != 0) {
        while (1) {
            if (strcmp(peekAt(0).type, "RESERVED_WORD") != 0 || !isTypeValue(peekAt(0).value)) {
                printf("Syntax Error at Line %d: Expected type in parameter list.\n", peekAt(0).line);
                fprintf(out, "Syntax Error at Line %d: Expected type in parameter list.\n", peekAt(0).line);
                errorCount++;
                recover();
                return;
            }
            
            char paramType[20];
            strncpy(paramType, peekAt(0).value, sizeof(paramType)-1);
            paramType[sizeof(paramType)-1] = '\0';
            match("RESERVED_WORD");

            if (strcmp(peekAt(0).type, "IDENTIFIER") != 0) {
                printf("Syntax Error at Line %d: Expected parameter name.\n", peekAt(0).line);
                fprintf(out, "Syntax Error at Line %d: Expected parameter name.\n", peekAt(0).line);
                errorCount++;
                recover();
                return;
            }

            declareIdentifier(peekAt(0).value, paramType, 0);

            if (funcId && funcId->paramCount < MAX_PARAMS) {
                strncpy(funcId->paramTypes[funcId->paramCount], paramType, 19);
                funcId->paramCount++;
            }

            match("IDENTIFIER");

            if (strcmp(peekAt(0).type, "COMMA") == 0) {
                match("COMMA");
                continue;
            } else break;
        }
    }
    match("RIGHT_PAREN");

    logTransition("parseFunctionDef", peekAt(0).value, "parseBlock");
    parseBlock();
    
    strcpy(currentFunctionReturnType, "none");
}

const char* parseExpression() {
    logTransition("parseExpression", peekAt(0).value, "parseLogicalOr");
    const char* type = parseLogicalOr();
    if (TRACK1) {
        printf("parseLogicalOr: DONE\n");
        fprintf(out, "parseLogicalOr:  DONE\n");
    }
    return type;
}

const char* parseLogicalOr() {
    logTransition("parseLogicalOr", peekAt(0).value, "parseLogicalAnd");
    const char* leftType = parseLogicalAnd();
    if (TRACK1) {
        printf("parseLogicalAnd: DONE\n");
        fprintf(out, "parseLogicalAnd: DONE\n");
    }
    while (strcmp(peekAt(0).type, "LOGICAL") == 0 && strcmp(peekAt(0).value, "or") == 0) {
        int line = peekAt(0).line;
        match("LOGICAL");
        logTransition("parseLogicalOr", peekAt(0).value, "parseLogicalAnd");
        const char* rightType = parseLogicalAnd();
        if (TRACK1) {
            printf("parseLogicalAnd: DONE\n");
            fprintf(out, "parseLogicalAnd: DONE\n");
        }


        if (strcmp(leftType, "bool") != 0 && strcmp(leftType, "unknown") != 0) {
            printf("Semantic Error at Line %d:  Logical 'or' requires boolean operands, got %s.\n", line, leftType);
            fprintf(out, "Semantic Error at Line %d: Logical 'or' requires boolean operands, got %s.\n", line, leftType);
            errorCount++;
        }
        if (strcmp(rightType, "bool") != 0 && strcmp(rightType, "unknown") != 0) {
            printf("Semantic Error at Line %d:  Logical 'or' requires boolean operands, got %s.\n", line, rightType);
            fprintf(out, "Semantic Error at Line %d: Logical 'or' requires boolean operands, got %s.\n", line, rightType);
            errorCount++;
        }
        leftType = "bool";
    }
    return leftType;
}

const char* parseLogicalAnd() {
    logTransition("parseLogicalAnd", peekAt(0).value, "parseLogicalNot");
    const char* leftType = parseLogicalNot();
    if (TRACK1) {
        printf("parseLogicalNot: DONE\n");
        fprintf(out, "parseLogicalNot: DONE\n");
    }
    while (strcmp(peekAt(0).type, "LOGICAL") == 0 && strcmp(peekAt(0).value, "and") == 0) {
        int line = peekAt(0).line;
        match("LOGICAL");
        logTransition("parseLogicalAnd", peekAt(0).value, "parseLogicalNot");
        const char* rightType = parseLogicalNot();
        if (TRACK1) {
            printf("parseLogicalNot: DONE\n");
            fprintf(out, "parseLogicalNot:  DONE\n");
        }


        if (strcmp(leftType, "bool") != 0 && strcmp(leftType, "unknown") != 0) {
            printf("Semantic Error at Line %d:  Logical 'and' requires boolean operands, got %s.\n", line, leftType);
            fprintf(out, "Semantic Error at Line %d: Logical 'and' requires boolean operands, got %s.\n", line, leftType);
            errorCount++;
        }
        if (strcmp(rightType, "bool") != 0 && strcmp(rightType, "unknown") != 0) {
            printf("Semantic Error at Line %d: Logical 'and' requires boolean operands, got %s.\n", line, rightType);
            fprintf(out, "Semantic Error at Line %d: Logical 'and' requires boolean operands, got %s.\n", line, rightType);
            errorCount++;
        }
        leftType = "bool";
    }
    return leftType;
}

const char* parseLogicalNot() {
    logTransition("parseLogicalNot", peekAt(0).value, "dispatching");
    if (strcmp(peekAt(0).type, "LOGICAL") == 0 && strcmp(peekAt(0).value, "not") == 0) {
        int line = peekAt(0).line;
        match("LOGICAL");
        logTransition("parseLogicalNot", peekAt(0).value, "parseLogicalNot");
        const char* operandType = parseLogicalNot();
        if (TRACK1) {
            printf("parseLogicalNot: DONE\n");
            fprintf(out, "parseLogicalNot: DONE\n");
        }


        if (strcmp(operandType, "bool") != 0 && strcmp(operandType, "unknown") != 0) {
            printf("Semantic Error at Line %d: Cannot use 'not' on %s.\n", line, operandType);
            fprintf(out, "Semantic Error at Line %d: Cannot use 'not' on %s.\n", line, operandType);
            errorCount++;
        }
        return "bool";
    }

    logTransition("parseLogicalNot", peekAt(0).value, "parseRelational");
    const char* type = parseRelational();
    if (TRACK1) {
        printf("parseRelational: DONE\n");
        fprintf(out, "parseRelational: DONE\n");
    }
    return type;
}

const char* parseRelational() {
    logTransition("parseRelational", peekAt(0).value, "parseArithmetic");
    const char* leftType = parseArithmetic();

    if (TRACK1) {
        printf("parseArithmetic: DONE\n");
        fprintf(out, "parseArithmetic: DONE\n");
    }


    if (strcmp(peekAt(0).type, "REL_OP") != 0) {

        return leftType;
    }


    while (strcmp(peekAt(0).type, "REL_OP") == 0) {
        match("REL_OP");
        logTransition("parseRelational", peekAt(0).value, "parseArithmetic");
        parseArithmetic();
        if (TRACK1) {
            printf("parseArithmetic: DONE\n");
            fprintf(out, "parseArithmetic: DONE\n");
        }
    }


    return "bool";
}

const char* parseArithmetic() {
    logTransition("parseArithmetic", peekAt(0).value, "parseTerm");
    const char* leftType = parseTerm();
    if (TRACK1) {
        printf("parseTerm: DONE\n");
        fprintf(out, "parseTerm: DONE\n");
    }

    while (strcmp(peekAt(0).type, "ARITH_OP") == 0 &&
          (strcmp(peekAt(0).value, "+") == 0 || strcmp(peekAt(0).value, "-") == 0)) {

        char op[10];
        strncpy(op, peekAt(0).value, sizeof(op) - 1);
        op[sizeof(op) - 1] = '\0';
        int line = peekAt(0).line;

        match("ARITH_OP");
        logTransition("parseArithmetic", peekAt(0).value, "parseTerm");
        const char* rightType = parseTerm();
        if (TRACK1) {
            printf("parseTerm:  DONE\n");
            fprintf(out, "parseTerm:  DONE\n");
        }

        // TYPE CHECK:  5 + "hello" should error
        if (strcmp(op, "+") == 0) {
            // String concatenation allowed
            if (strcmp(leftType, "str") == 0 && strcmp(rightType, "str") == 0) {
                leftType = "str";
            } else if (strcmp(leftType, "str") == 0 || strcmp(rightType, "str") == 0) {
                printf("Semantic Error at Line %d: Cannot add %s and %s.\n", line, leftType, rightType);
                fprintf(out, "Semantic Error at Line %d: Cannot add %s and %s.\n", line, leftType, rightType);
                errorCount++;
            } else {
                // Numeric addition - promote to float if needed
                if (strcmp(leftType, "float") == 0 || strcmp(rightType, "float") == 0) {
                    leftType = "float";
                }
            }
        } else if (strcmp(op, "-") == 0) {
            // Subtraction only works on numbers
            if (strcmp(leftType, rightType) != 0) {
                printf("Semantic Error at Line %d: Cannot subtract %s and %s.\n", line, leftType, rightType);
                fprintf(out, "Semantic Error at Line %d: Cannot subtract %s and %s.\n", line, leftType, rightType);
                errorCount++;
            } else if (strcmp(leftType, "float") == 0 || strcmp(rightType, "float") == 0) {
                leftType = "float";
            }
        }
    }

    return leftType;
}

const char* parseTerm() {
    logTransition("parseTerm", peekAt(0).value, "parseFactor");
    const char* leftType = parseFactor();
    if (TRACK1) {
        printf("parseFactor: DONE\n");
        fprintf(out, "parseFactor: DONE\n");
    }

    while (strcmp(peekAt(0).type, "ARITH_OP") == 0 &&
          (strcmp(peekAt(0).value, "*") == 0 ||
           strcmp(peekAt(0).value, "/") == 0 ||
           strcmp(peekAt(0).value, "%") == 0 ||
           strcmp(peekAt(0).value, "//") == 0)) {

        char op[10];
        strncpy(op, peekAt(0).value, sizeof(op) - 1);
        op[sizeof(op) - 1] = '\0';
        int line = peekAt(0).line;

        match("ARITH_OP");
        logTransition("parseTerm", peekAt(0).value, "parseFactor");
        const char* rightType = parseFactor();
        if (TRACK1) {
            printf("parseFactor:  DONE\n");
            fprintf(out, "parseFactor:  DONE\n");
        }

        // TYPE CHECK:  Cannot multiply/divide incompatible types
        if (strcmp(leftType, "str") == 0 || strcmp(rightType, "str") == 0) {
            printf("Semantic Error at Line %d: Cannot use operator '%s' with string type.\n", line, op);
            fprintf(out, "Semantic Error at Line %d: Cannot use operator '%s' with string type.\n", line, op);
            errorCount++;
        } else if (strcmp(leftType, "bool") == 0 || strcmp(rightType, "bool") == 0) {
            printf("Semantic Error at Line %d: Cannot use operator '%s' with boolean type.\n", line, op);
            fprintf(out, "Semantic Error at Line %d: Cannot use operator '%s' with boolean type.\n", line, op);
            errorCount++;
        }

        // Result type promotion:  if either is float, result is float
        if (strcmp(leftType, "float") == 0 || strcmp(rightType, "float") == 0) {
            leftType = "float";
        }
    }

    return leftType;
}

const char* parseFactor() {
    logTransition("parseFactor", peekAt(0).value, "dispatching");
    const char* currentType = "unknown";

    if (strcmp(peekAt(0).type, "ARITH_OP") == 0 &&
        (strcmp(peekAt(0).value, "+") == 0 || strcmp(peekAt(0).value, "-") == 0)) {
        match("ARITH_OP");
        logTransition("parseFactor", peekAt(0).value, "parseFactor");

        const char* type = parseFactor();
        if (TRACK1) {
            printf("parseFactor:  DONE\n");
            fprintf(out, "parseFactor:  DONE\n");
        }
        return type;
    }

    if (strcmp(peekAt(0).type, "INCREMENT") == 0 || strcmp(peekAt(0).type, "DECREMENT") == 0) {
        match(peekAt(0).type);
        match("IDENTIFIER");
        return "int";
    }

    if (strcmp(peekAt(0).type, "NUMBER") == 0) {
        const char* numVal = peekAt(0).value;
        advance();
        if (strchr(numVal, '.') != NULL) {
            currentType = "float";
        } else {
            currentType = "int";
        }
    }

    else if (strcmp(peekAt(0).type, "STRING") == 0 || strcmp(peekAt(0).type, "STRING_INTERP") == 0) {
        advance();
        currentType = "str";
    }

    else if (strcmp(peekAt(0).type, "CHAR") == 0) {
        advance();
        currentType = "char";
    }

    else if (strcmp(peekAt(0).type, "IDENTIFIER") == 0) {

        if (strcmp(peekAt(1).type, "LEFT_PAREN") == 0) {
            logTransition("parseFactor", peekAt(0).value, "parseFunctionCall");
            parseFunctionCall();
            if (TRACK1) {
                printf("parseFunctionCall: DONE\n");
                fprintf(out, "parseFunctionCall: DONE\n");
            }
            currentType = "unknown";
        }
        // Check for Variable
        else {
            const char* varName = peekAt(0).value;
            if (!isIdentifierDeclared(varName)) {
                printf("Semantic Error at Line %d: Identifier '%s' is not declared\n", peekAt(0).line, varName);
                fprintf(out, "Semantic Error at Line %d: Identifier '%s' is not declared\n", peekAt(0).line, varName);
                errorCount++;
                match("IDENTIFIER");
                currentType = "unknown";
            } else {
                currentType = getIdentifierType(varName);
                match("IDENTIFIER");

                if (strcmp(peekAt(0).type, "INCREMENT") == 0 || strcmp(peekAt(0).type, "DECREMENT") == 0) {
                    advance();
                }
            }
        }
    }

    else if (strcmp(peekAt(0).type, "LEFT_PAREN") == 0) {
        match("LEFT_PAREN");
        logTransition("parseFactor", peekAt(0).value, "parseExpression");
        currentType = parseExpression(); // Recurse back to top
        if (TRACK1) {
            printf("parseExpression:  DONE\n");
            fprintf(out, "parseExpression:  DONE\n");
        }
        if (strcmp(peekAt(0).type, "RIGHT_PAREN") != 0) {
            printf("Syntax Error at Line %d: Missing closing parenthesis\n", peekAt(0).line);
            fprintf(out, "Syntax Error at Line %d: Missing closing parenthesis\n", peekAt(0).line);
            errorCount++;
            logTransition("parseFactor", peekAt(0).value, "errorRecovery");
            recover();
            return "unknown";
        }
        match("RIGHT_PAREN");
    }

    else if (strcmp(peekAt(0).type, "RESERVED_WORD") == 0 &&
        (strcmp(peekAt(0).value, "True") == 0 || strcmp(peekAt(0).value, "False") == 0)) {
        advance();
        currentType = "bool";
    }

    else if (strcmp(peekAt(0).type, "RESERVED_WORD") == 0 && strcmp(peekAt(0).value, "null") == 0) {
        advance();
        currentType = "null";
    }

    else {
        printf("Syntax Error at Line %d: Missing operand - Expected NUMBER, IDENTIFIER, STRING, or '(' but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
        fprintf(out, "Syntax Error at Line %d: Missing operand - Expected NUMBER, IDENTIFIER, STRING, or '(' but got %s (%s)\n", peekAt(0).line, peekAt(0).type, peekAt(0).value);
        errorCount++;
        logTransition("parseFactor", peekAt(0).value, "errorRecovery");
        recover();
        return "unknown";
    }

    if (strcmp(peekAt(0).type, "ARITH_OP") == 0 && strcmp(peekAt(0).value, "**") == 0) {
        int line = peekAt(0).line;
        match("ARITH_OP");

        logTransition("parseFactor", peekAt(0).value, "parseFactor");
        const char* rightType = parseFactor();

        if (TRACK1) {
            printf("parseFactor: DONE\n");
            fprintf(out, "parseFactor: DONE\n");
        }

        if (strcmp(currentType, "str") == 0 || strcmp(rightType, "str") == 0) {
            printf("Semantic Error at Line %d: Cannot use exponentiation on string.\n", line);
            fprintf(out, "Semantic Error at Line %d: Cannot use exponentiation on string.\n", line);
            errorCount++;
        }

        if (strcmp(currentType, "float") == 0 || strcmp(rightType, "float") == 0) {
            return "float";
        }
        return "int";
    }

    return currentType;
}

int main() {
    printf("Running Lexer...\n");
    int result = system("gcc FSM_lexical_analyzer.c -o lexer && lexer");

    if (result != 0) {
        printf("Error: Lexer failed to run.\n");
        return 1;
    }
    printf("Lexer finished. Starting Parser...\n\n");
    tokenFile = fopen("Symbol_Table.txt", "r");
    if (!tokenFile) {
        printf("Error: Cannot open Symbol_Table.txt\n");
        return 1;
    }

    out = fopen("Output_Syntax.txt", "w");
    if (!out) {
        printf("Error: Cannot create output file.\n");
        fclose(tokenFile);
        return 1;
    }

    errorCount = 0;
    braceBalance = 0;

    initLookahead();
    parseProgram();
    if (TRACK1) {
        printf("parseProgram: DONE\n");
        fprintf(out, "parseProgram: DONE\n");
    }

    fclose(tokenFile);

    if (errorCount > 0) {
        printf("Parsing finished. Total errors: %d\n", errorCount);
        fprintf(out, "Parsing finished. Total errors: %d\n", errorCount);
    } else {
        printf("Parsing finished successfully.\n");
        fprintf(out, "Parsing finished successfully.\n");
    }

    return 0;
}
Token getNextToken() {
    char line[256];
    Token t = { "EOF", "", lines };
    while (fgets(line, sizeof(line), tokenFile)) {

        if (strlen(line) < 3) continue;
        line[strcspn(line, "\n")] = '\0';

        char *open = strchr(line, '(');
        char *close = strrchr(line, ')');

        if (!open || !close || close < open) continue;

        int typeLen = open - line;
        strncpy(t.type, line, typeLen);
        t.type[typeLen] = '\0';

        if (strcmp(t.type, "COMMENT") == 0 ||
            strcmp(t.type, "COMMENT_START") == 0 ||
            strcmp(t.type, "COMMENT_END") == 0 ||
            strcmp(t.type, "COM_STR") == 0) {
            continue;
            }

        if (strcmp(t.type, "NEW_LINE") == 0) {
            lines++;
            continue;
        }

        int valLen = close - open - 1;
        if (valLen > 0) {
            strncpy(t.value, open + 1, valLen);
            t.value[valLen] = '\0';
        } else {
            t.value[0] = '\0';
        }
        t.line = lines;
        return t;
    }
    EOF_TOKEN.line = lines; 
    return EOF_TOKEN;
}