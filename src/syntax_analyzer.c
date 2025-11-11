#include <stdio.h>
#include <string.h>

#define MAX_TOKENS 5000

typedef struct {
    char type[50];
    char value[100];
} Token;

Token tokens[MAX_TOKENS];
int tokenCount = 0;
int current = 0;

Token peek() {
    return tokens[current];
}

void advance() {
    if (current < tokenCount - 1) current++;
}

int match(const char* expected) {
    if (strcmp(peek().type, expected) == 0) {
        advance();
        return 1;
    }

    printf("Syntax Error: Expected %s but got %s (%s)\n",
           expected, peek().type, peek().value);

    recover();
    return 0;
}

void recover() {

    while (current < tokenCount &&
           strcmp(tokens[current].type, "SEMICOLON") != 0 &&
           strcmp(tokens[current].type, "RIGHT_BRACE") != 0) {
        current++;
    }

    if (current < tokenCount) current++;
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
void parseIf();
void parseWhile();
void parseFor();
void parseAssignment();
void parseExpression();
void parseTerm();
void parseFactor();

void parseProgram() {
    int start = findMain();
    if (start == -1) {
        printf("Syntax Error: No main() function found.\n");
        return;
    }

    current = start;

    match("KEYWORD");
    match("LEFT_PAREN");
    match("RIGHT_PAREN");

    parseBlock();
}

//Kaya niyo yan gaiz
void parseBlock() {
    match("LEFT_BRACE");

    while (current < tokenCount &&
           strcmp(peek().type, "RIGHT_BRACE") != 0) {

        parseStatement();
    }

    match("RIGHT_BRACE");
}

void parseStatement() {

    if (strcmp(peek().type, "KEYWORD") == 0 &&
        strcmp(peek().value, "if") == 0) {
        parseIf();
        return;
    }

    if (strcmp(peek().type, "KEYWORD") == 0 &&
        strcmp(peek().value, "while") == 0) {
        parseWhile();
        return;
    }

    if (strcmp(peek().type, "KEYWORD") == 0 &&
        strcmp(peek().value, "for") == 0) {
        parseFor();
        return;
    }

    if (strcmp(peek().type, "IDENTIFIER") == 0) {
        parseAssignment();
        return;
    }

    printf("Syntax Error: Unknown statement starting with %s\n",
           peek().type);
    recover();
}

void parseIf() {
    match("KEYWORD");
    match("LEFT_PAREN");
    parseExpression();
    match("RIGHT_PAREN");

    parseBlock();

    if (strcmp(peek().type, "KEYWORD") == 0 &&
        strcmp(peek().value, "else") == 0) {
        advance();
        parseBlock();
    }
}

void parseWhile() {
    match("KEYWORD");
    match("LEFT_PAREN");
    parseExpression();
    match("RIGHT_PAREN");
    parseBlock();
}

void parseFor() {
    match("KEYWORD");
    match("LEFT_PAREN");

    match("IDENTIFIER");
    match("ASSIGN_OP");
    parseExpression();
    match("SEMICOLON");

    parseExpression();
    match("SEMICOLON");

    match("IDENTIFIER");
    match("ASSIGN_OP");
    parseExpression();

    match("RIGHT_PAREN");
    parseBlock();
}

void parseAssignment() {
    match("IDENTIFIER");
    match("ASSIGN_OP");
    parseExpression();
    match("SEMICOLON");
}

void parseExpression() {
    parseTerm();
    while (strcmp(peek().type, "ARITH_OP") == 0 &&
          (strcmp(peek().value, "+") == 0 ||
           strcmp(peek().value, "-") == 0)) {

        advance();
        parseTerm();
    }
}

void parseTerm() {
    parseFactor();
    while (strcmp(peek().type, "ARITH_OP") == 0 &&
          (strcmp(peek().value, "*") == 0 ||
           strcmp(peek().value, "/") == 0)) {

        advance();
        parseFactor();
    }
}

void parseFactor() {
    if (strcmp(peek().type, "NUMBER") == 0) {
        advance();
        return;
    }

    if (strcmp(peek().type, "IDENTIFIER") == 0) {
        advance();
        return;
    }

    if (strcmp(peek().type, "LEFT_PAREN") == 0) {
        advance();
        parseExpression();
        match("RIGHT_PAREN");
        return;
    }

    printf("Syntax Error: Expected NUMBER, IDENTIFIER, or '('\n");
    recover();
}

int main() {
    FILE *fp = fopen("Symbol_Table.txt", "r");
    if (!fp) {
        printf("Error: Cannot open Symbol_Table.txt\n");
        return 1;
    }

    char line[256];

    while (fgets(line, sizeof(line), fp) != NULL) {

        char *open = strchr(line, '(');
        char *close = strrchr(line, ')');

        if (!open || !close || close < open) {
            continue;
        }

        int typeLength = open - line;
        strncpy(tokens[tokenCount].type, line, typeLength);
        tokens[tokenCount].type[typeLength] = '\0';

        int valueLength = close - open - 1;
        strncpy(tokens[tokenCount].value, open + 1, valueLength);
        tokens[tokenCount].value[valueLength] = '\0';

        tokenCount++;
        if (tokenCount >= MAX_TOKENS) break;
    }

    fclose(fp);
    parseProgram();
    return 0;
}