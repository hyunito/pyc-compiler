#include <stdio.h>
#include <ctype.h>
#include <string.h>

int main() {
    FILE *fp = fopen("../test/test_error.pyclang", "r");
    if (!fp) {
        printf("Error: could not open file.\n");
        return 1;
    }

    char c;
    int state = 0;
    char buffer[999];
    int idx = 0;

    while ((c = fgetc(fp)) != EOF) {
        switch(state) {
            case 0:
                if (isalpha(c)) { 
                    buffer[idx++] = c;
                    state = 1;
                } else if (isdigit(c)) { 
                    buffer[idx++] = c;
                    state = 2;
                } else if (c == '=') {
                    int next = fgetc(fp);
                    if (next == '=') {
                        printf("REL_OP(==)\n");
                    } else {
                        printf("ASSIGN_OP(=)\n");
                        ungetc(next, fp);
                    }
                } else if (c == '#') {
                    printf("COMMENT(#)\n");
                    state = 4;
                } else if (c == '/'){
                    int next = fgetc(fp);
                    if (next == '*') {
                        printf("COMMENT_START(/*)\n");
                        state = 5;
                    } else {
                        printf("DIV(/)\n");
                        ungetc(next, fp);
                    }
                } else if (c == ';') {
                    printf("SEMICOLON(;)\n");
                } else if (c == '(') {
                    printf("LEFT_PAREN(()\n");
                } else if (c == ')') {
                    printf("RIGHT_PAREN())\n");
                } else if (c == '<') {
                    int next = fgetc(fp);
                    if (next == '=') {
                        printf("REL_OP(<=)\n");
                    } else {
                        printf("REL_OP(<)\n");
                        ungetc(next, fp);
                    }
                } else if (c == '>') {
                    int next = fgetc(fp);
                    if (next == '=') {
                        printf("REL_OP(>=)\n");
                    } else {
                        printf("REL_OP(>)\n");
                        ungetc(next, fp);
                    }
                } else if (c == '{') {
                    printf("LEFT_BRACE({)\n");
                } else if (c == '}') {
                    printf("RIGHT_BRACE(})\n");
                } else if (c == '[') {
                    printf("LEFT_BRACKET([)\n");
                } else if (c == ']') {
                    printf("RIGHT_BRACKET(])\n");
                } else if (c == ',') {
                    printf("COMMA(,)\n");
                } else if (c == '+') {
                    int next = fgetc(fp);
                    if (next == '+') {
                        printf("INCREMENT(++)\n");
                    } else {
                        printf("ADD(+)\n");
                        ungetc(next, fp);
                    }
                } else if (c == '-') {
                    int next = fgetc(fp);
                    if (next == '-') {
                        printf("DECREMENT(--)\n");
                    } else {
                        printf("SUB(-)\n");
                        ungetc(next, fp);
                    }
                } else if (isspace(c)) {

                } else if (c == '"') {
                    state = 3; 
                } else {
                    printf("ERROR(%c)\n", c);
                }
                break;

            case 1:
                if (isalnum(c)) {
                    buffer[idx++] = c;
                } else {
                    buffer[idx] = '\0';
                    if (strcmp(buffer, "output") == 0 || strcmp(buffer, "input") == 0 || strcmp(buffer, "if") == 0 || strcmp(buffer, "else") == 0 || strcmp(buffer, "while") == 0 || strcmp(buffer, "for") == 0 || strcmp(buffer, "break") == 0 || strcmp(buffer, "continue") == 0 || strcmp(buffer, "return") == 0) {
                        printf("KEYWORD(%s)\n", buffer);
                    } else if (strcmp(buffer, "int") == 0 || strcmp(buffer, "float") == 0 || strcmp(buffer, "bool") == 0 || strcmp(buffer, "char") == 0 || strcmp(buffer, "str") == 0 || strcmp(buffer, "void") == 0 || strcmp(buffer, "True") == 0 || strcmp(buffer, "False") == 0) {
                        printf("RESERVED_WORD(%s)\n", buffer);
                    } else if (strcmp(buffer, "and") == 0 || strcmp(buffer, "or") == 0 || strcmp(buffer, "not") == 0) {
                        printf("LOGICAL(%s)\n", buffer);
                    } else
                        printf("IDENTIFIER(%s)\n", buffer);
                    idx = 0;
                    state = 0;
                    ungetc(c, fp);
                }
                break;

            case 2:
                if (isdigit(c)) {
                    buffer[idx++] = c;
                } else if (c == '.') {
                    buffer[idx++] = c;
                } else {
                    buffer[idx] = '\0';
                    printf("NUMBER(%s)\n", buffer);
                    idx = 0;
                    state = 0;
                    ungetc(c, fp);
                }
                break;

            case 3:
                if (c != '"') {
                    buffer[idx++] = c;
                } else {
                    buffer[idx] = '\0';
                    printf("STRING(%s)\n", buffer);
                    idx = 0;
                    state = 0;
                }
                break;

            case 4:
                if (c != '\n') {
                    buffer[idx++] = c;
                } else {
                    buffer[idx] = '\0';
                    printf("COM_STR(%s)\n", buffer);
                    idx = 0;
                    state = 0;
                }
                break;

            case 5:
                if (c == '*') {
                    int next = fgetc(fp);
                    if (next == '/') {
                        buffer[idx] = '\0';
                        printf("COM_STR(%s)\n", buffer);
                        printf("COMMENT_END(*/)\n");
                        idx = 0;
                        state = 0;
                    } else {
                        buffer[idx++] = c;
                        ungetc(next, fp);
                        break;
                    }
                } else {
                    buffer[idx++] = c;
                }
                break;
        }
    }

    fclose(fp);
    return 0;
}
