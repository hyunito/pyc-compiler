#include <stdio.h>
#include <string.h>

#define MAX_TOKENS 5000
#define LOGGING 0  // 1 to enable, 0 to disable
#define DEBUG 0 // 1 to enable, 0 to disable

typedef struct {
    char type[50];
    char value[100];
} Token;

Token tokens[MAX_TOKENS];
int tokenCount = 0;
int current = 0;
int errorCount = 0;

static Token EOF_TOKEN = { "EOF", "" };

int isAtEnd() {
    return current >= tokenCount;
}

Token peek() {
    if (LOGGING) {
        printf("[LOG] peek(): Returning %s at position %d\n", 
               isAtEnd() ? "EOF" : tokens[current].type, current);
    }

    if (isAtEnd()) return EOF_TOKEN;
    return tokens[current];
}

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
        printf("[DEBUG] advance(): Moving from %s('%s') to ", 
               tokens[current].type, tokens[current].value);
    }

    if (!isAtEnd()) current++;

    if (DEBUG) {
        if (!isAtEnd()) 
            printf("%s('%s')\n", tokens[current].type, tokens[current].value);
        else 
            printf("EOF\n");
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
void parseDeclaration();

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
    if (strcmp(peek().type, "RESERVED_WORD") == 0) {
        parseDeclaration();
        return;
    }

    printf("Syntax Error: Unknown statement starting with %s\n",
           peek().type);
    recover();
}
void parseDeclaration() {
    match("RESERVED_WORD");
    match("IDENTIFIER");

    if (strcmp(peek().type, "ASSIGN_OP") == 0) {
        match("ASSIGN_OP");
        parseExpression();
    }
    match("SEMICOLON");
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
    errorCount++;
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

        if (strlen(line) < 3) {
            continue;
        }

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