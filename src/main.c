#include <stdio.h>
#include <ctype.h>
#include <string.h>

int main() {
    FILE *fp = fopen("../test/test_basic.pyclang", "r");
    if (!fp) {
        printf("Error: could not open file.\n");
        return 1;
    }

    char c;
    int state = 0;
    char buffer[100];
    int idx = 0;

    while ((c = fgetc(fp)) != EOF) {
        switch (state) {
            case 0:
                if (isalpha(c)) { 
                    buffer[idx++] = c;
                    state = 1;
                } else if (isdigit(c)) { 
                    buffer[idx++] = c;
                    state = 2;
                } else if (c == '=') {
                    printf("ASSIGN_OP(=)\n");
                } else if (c == ';') {
                    printf("SEMICOLON(;)\n");
                } else if (c == '(') {
                    printf("LEFT_PAREN\n");
                } else if (c == ')') {
                    printf("RIGHT_PAREN\n");
                } else if (c == '>') {
                    printf("REL_OP(>)\n");
                } else if (c == '{') {
                    printf("LEFT_BRACE({)\n");
                } else if (c == '}') {
                    printf("RIGHT_BRACE(})\n");
                } else if (isspace(c)) {
                   
                } else if (c == '"') {
                    state = 3; 
                } else {
                    printf("UNKNOWN(%c)\n", c);
                }
                break;

            case 1:
                if (isalnum(c)) {
                    buffer[idx++] = c;
                } else {
                    buffer[idx] = '\0';
                    if (strcmp(buffer, "int") == 0 || strcmp(buffer, "float") == 0 || strcmp(buffer, "output") == 0 || strcmp(buffer, "bool") == 0 || strcmp(buffer, "True") == 0 || strcmp(buffer, "if") == 0 || strcmp(buffer, "else") == 0)
                        printf("KEYWORD(%s)\n", buffer);
                    else
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
        }
    }

    fclose(fp);
    return 0;
}
