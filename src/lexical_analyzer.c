#include <stdio.h>
#include <ctype.h>

int str_equal(const char *a, const char *b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i])
            return 0;
        i++;
    }
    return (a[i] == '\0' && b[i] == '\0');
}

const char *keywords[] = {
    "main","output","input","if","else",
    "while","for","break","continue","return"
};

const char *reserved[] = {
    "int","float","bool","char","str",
    "void","True","False"
};

const char *logical_ops[] = {
    "and","or","not"
};

const char *noise_words[] = {
    "then","begin","end","do"
};

int keyword(const char *word) {
    int count = sizeof(keywords)/sizeof(keywords[0]);
    for (int i = 0; i < count; i++)
        if (str_equal(word, keywords[i])) return 1;
    return 0;
}

int reserve(const char *word) {
    int count = sizeof(reserved)/sizeof(reserved[0]);
    for (int i = 0; i < count; i++)
        if (str_equal(word, reserved[i])) return 1;
    return 0;
}

int logical(const char *word) {
    int count = sizeof(logical_ops)/sizeof(logical_ops[0]);
    for (int i = 0; i < count; i++)
        if (str_equal(word, logical_ops[i])) return 1;
    return 0;
}

int noise(const char *word) {
    int count = sizeof(noise_words)/sizeof(noise_words[0]);
    for (int i = 0; i < count; i++)
        if (str_equal(word, noise_words[i])) return 1;
    return 0;
}

int main() {
    char filename[] = "sample.pyclang";

    int len = 0;
    while (filename[len] != '\0') {
        len++;
    }
    if (len < 8 || !str_equal(filename + (len - 8), ".pyclang")) {
        printf("Error: Invalid file type. Must be .pyclang\n");
        return 1;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Cannot open input file.\n");
        return 1;
    }

    FILE *out = fopen("Symbol_Table.txt", "w");
    if (!out) {
        printf("Error: Cannot create output file.\n");
        fclose(fp);
        return 1;
    }

    char c;
    int state = 0;
    char buffer[999];
    int idx = 0;

    while ((c = fgetc(fp)) != EOF) {

        switch (state) {

        case 0:
            if (isalpha(c)) {
                buffer[idx++] = c;
                state = 1;
            }
            else if (isdigit(c)) {
                int next = fgetc(fp);
                if (isalpha(next)) {
                    printf("ERROR(%c)\n", c);
                    fprintf(out, "ERROR(%c)\n", c);
                    ungetc(next, fp);
                    break;
                }
                ungetc(next, fp);
                buffer[idx++] = c;
                state = 2;
            }
            else if (c == '=') {
                int next = fgetc(fp);
                if (next == '=') {
                    printf("REL_OP(==)\n");
                    fprintf(out, "REL_OP(==)\n");
                } else {
                    printf("ASSIGN_OP(=)\n");
                    fprintf(out, "ASSIGN_OP(=)\n");
                    ungetc(next, fp);
                }
            }
            else if (c == '#') {
                printf("COMMENT(#)\n");
                fprintf(out, "COMMENT(#)\n");
                state = 4;
            }
            else if (c == '/') {
                int next = fgetc(fp);
                if (next == '*') {
                    printf("COMMENT_START(/*)\n");
                    fprintf(out, "COMMENT_START(/*)\n");
                    state = 5;
                }
                else if (next == '/') {
                    printf("ARITH_OP(//)\n");
                    fprintf(out, "ARITH_OP(//)\n");
                }
                else {
                    printf("ARITH_OP(/)\n");
                    fprintf(out, "ARITH_OP(/)\n");
                    ungetc(next, fp);
                }
            }
            else if (c == ';') { printf("SEMICOLON(;)\n"); fprintf(out, "SEMICOLON(;)\n"); }
            else if (c == '(') { printf("LEFT_PAREN(()\n"); fprintf(out, "LEFT_PAREN(()\n"); }
            else if (c == ')') { printf("RIGHT_PAREN())\n"); fprintf(out, "RIGHT_PAREN())\n"); }
            else if (c == '!') {
                int next = fgetc(fp);
                if (next == '=') {
                    printf("REL_OP(!=)\n");
                    fprintf(out, "REL_OP(!=)\n");
                } else {
                    printf("ERROR(!)\n");
                    fprintf(out, "ERROR(!)\n");
                    ungetc(next, fp);
                }
            }
            else if (c == '<') {
                int next = fgetc(fp);
                if (next == '=') {
                    printf("REL_OP(<=)\n");
                    fprintf(out, "REL_OP(<=)\n");
                } else {
                    printf("REL_OP(<)\n");
                    fprintf(out, "REL_OP(<)\n");
                    ungetc(next, fp);
                }
            }
            else if (c == '>') {
                int next = fgetc(fp);
                if (next == '=') {
                    printf("REL_OP(>=)\n");
                    fprintf(out, "REL_OP(>=)\n");
                } else {
                    printf("REL_OP(>)\n");
                    fprintf(out, "REL_OP(>)\n");
                    ungetc(next, fp);
                }
            }
            else if (c == '{') { printf("LEFT_BRACE({)\n"); fprintf(out, "LEFT_BRACE({)\n"); }
            else if (c == '}') { printf("RIGHT_BRACE(})\n"); fprintf(out, "RIGHT_BRACE(})\n"); }
            else if (c == '[') { printf("LEFT_BRACKET([)\n"); fprintf(out, "LEFT_BRACKET([)\n"); }
            else if (c == ']') { printf("RIGHT_BRACKET(])\n"); fprintf(out, "RIGHT_BRACKET(])\n"); }
            else if (c == ',') { printf("COMMA(,)\n"); fprintf(out, "COMMA(,)\n"); }

            else if (c == '+') {
                int next = fgetc(fp);
                if (next == '+') { printf("INCREMENT(++)\n"); fprintf(out, "INCREMENT(++)\n"); }
                else { printf("ARITH_OP(+)\n"); fprintf(out, "ARITH_OP(+)\n"); ungetc(next, fp); }
            }
            else if (c == '-') {
                int next = fgetc(fp);
                if (next == '-') { printf("DECREMENT(--)\n"); fprintf(out, "DECREMENT(--)\n"); }
                else { printf("ARITH_OP(-)\n"); fprintf(out, "ARITH_OP(-)\n"); ungetc(next, fp); }
            }
            else if (c == '*') {
                int next = fgetc(fp);
                if (next == '*') { printf("ARITH_OP(**)\n"); fprintf(out, "ARITH_OP(**)\n"); }
                else { printf("ARITH_OP(*)\n"); fprintf(out, "ARITH_OP(*)\n"); ungetc(next, fp); }
            }
            else if (c == '%') { printf("ARITH_OP(%%)\n"); fprintf(out, "ARITH_OP(%%)\n"); }
            else if (c == '\n') {
                printf("NEW_LINE(\\n)\n");
                fprintf(out, "NEW_LINE(\\n)\n");
            } else if (isspace(c)) {}
            else if (c == '"') { state = 3; }
            else if (c == '\'') { state = 6; }
            else {
                printf("ERROR(%c)\n", c);
                fprintf(out, "ERROR(%c)\n", c);
            }
            break;

        case 1:
            if (isalnum(c) || c == '_') {
                buffer[idx++] = c;
            } else {
                buffer[idx] = '\0';
                if (keyword(buffer)) {
                    printf("KEYWORD(%s)\n", buffer);
                    fprintf(out, "KEYWORD(%s)\n", buffer);
                }
                else if (reserve(buffer)) {
                    printf("RESERVED_WORD(%s)\n", buffer);
                    fprintf(out, "RESERVED_WORD(%s)\n", buffer);
                }
                else if (logical(buffer)) {
                    printf("LOGICAL(%s)\n", buffer);
                    fprintf(out, "LOGICAL(%s)\n", buffer);
                }
                else if (noise(buffer)) {
                    printf("NOISE_WORD(%s)\n", buffer);
                    fprintf(out, "NOISE_WORD(%s)\n", buffer);
                }
                else {
                    if (c != '!' && c != '}' && c != '@' && c != '#' && c != '$' && c != '^' && c != ')' && c != '&' && c != '|' && c != '\\'){
                        printf("IDENTIFIER(%s)\n", buffer);
                        fprintf(out,"IDENTIFIER(%s)\n", buffer);
                    } else {
                        printf("ERROR(%c)\n", c);
                        fprintf(out,"ERROR(%c)\n", c);
                        c = fgetc(fp);
                    }
                }

                idx = 0;
                state = 0;
                ungetc(c, fp);
            }
            break;

        case 2:
            if (isdigit(c) || c == '.') {
                buffer[idx++] = c;
            } else {
                buffer[idx] = '\0';
                printf("NUMBER(%s)\n", buffer);
                fprintf(out, "NUMBER(%s)\n", buffer);
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
                fprintf(out, "STRING(%s)\n", buffer);
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
                fprintf(out, "COM_STR(%s)\n", buffer);
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
                    fprintf(out, "COM_STR(%s)\n", buffer);
                    printf("COMMENT_END(*/)\n");
                    fprintf(out, "COMMENT_END(*/)\n");
                    idx = 0;
                    state = 0;
                } else {
                    buffer[idx++] = c;
                    ungetc(next, fp);
                }
            } else {
                buffer[idx++] = c;
            }
            break;
         case 6:
                int next = fgetc(fp);
                if (next == '\'') {
                    buffer[idx] = '\0';
                    printf("CHAR(%c)\n", c);
                    fprintf(out, "CHAR(%c)\n", c);
                    idx = 0;
                    state = 0;
                } else {
                    printf("ERROR(%c)\n", c);
                    fprintf(out,"ERROR(%c)\n", c);
                    ungetc(next, fp);
                }
        }
    }
    fclose(fp);
    fclose(out);
    return 0;
}
