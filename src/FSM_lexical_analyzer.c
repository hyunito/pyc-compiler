#include <stdio.h>

char cap[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

char sm[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

char symbols[] = {
    '!', '#', '^', '*', '%', '&', '(', ')', '[', ']', '{', '}', '<', '>',
    '+', '=', '-', '|', '/', ';', ':', '\'', '"', ',', '.', '_', '$', '@', '~', '\\', '?', '^'
};

char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
int lensm = sizeof(sm) / sizeof(sm[0]);
int lensymbols = sizeof(symbols) / sizeof(symbols[0]);

int str_equal(const char *str1, const char *str2) {
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return 0;
        }
        i++;
    }
    return (str1[i] == '\0' && str2[i] == '\0');
}

int isLetter(char letter1, char letter2[]) {
    for (int i = 0; i < lensm; i++) {
        if (letter1 == letter2[i]) {
            return 1;
        }
        }
    return 0;
}
int isDigit(char num) {
    for (int i = 0; i < 10; i++) {
        if (num == digits[i]) {
            return 1;
        }
    }
    return 0;
}

int isSymbol(char symbol1) {
    for (int i = 0; i < lensymbols; i++) {
        if (symbol1 == symbols[i]) {
            return 1;
        }
    }
    return 0;
}

int main() {
    char filename[] = "syntax_input.pyclang";

    int len = 0;
    while (filename[len] != '\0') len++;

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
    int exit = 0;
    char c;
    int state = 0;
    char buffer[999];
    int idx = 0;

    while ((c = fgetc(fp)) != EOF) {

        //printf("DEBUG: state = %d, char = '%c'(ASCII = %d), idx = %d\n", state, c, c, idx);
        switch (state) {

            //Starting state 0
            case 0:
                // Skip all whitespace except newlines
                if (c == ' ' || c == '\t') {
                    break;
                } else if (c == '\n') {
                    printf("NEW_LINE(\\n)\n");
                    fprintf(out, "NEW_LINE(\\n)\n");
                    break;
                } else if (c == EOF || c == -1) {
                    break;
                }

                //Go to State 1 if small letters
                if (isLetter(c, sm)) {
                    state = 1;
                    ungetc(c, fp);
                    break;
                } else if (isLetter(c, cap)) {
                    //Go to State 2 if capital letters
                    state = 2;
                    ungetc(c, fp);
                    break;
                } else if (isDigit(c)) {
                    //Go to State 3 if digits
                    state = 3;
                    ungetc(c, fp);
                    break;
                } else if (isSymbol(c)) {
                    //Go to State 4 if symbols
                    state = 4;
                    ungetc(c, fp);
                    break;
                } else {
                    // Unrecognized character
                    buffer[idx++] = c;
                    state = 113;
                    break;
                }

            //State 1 is for small letters only this includes small letters
            //in Keywords, Reserved Words, Noise Words
            case 1:
                if (c == 'i') {
                    //Keywords: if, input,
                    //Reserve Words: int
                    buffer[idx++] = c;
                    state = 5;
                    break;
                } else if (c == 'e') {
                    //Keywords: else
                    //Noise Words: end
                    buffer[idx++] = c;
                    state = 6;
                    break;
                } else if (c == 'f') {
                    //Keywords: for
                    //Reserve Words: float
                    buffer[idx++] = c;
                    state = 7;
                    break;
                } else if (c == 'w') {
                    //Keywords: while
                    buffer[idx++] = c;
                    state = 8;
                    break;
                } else if (c == 'b') {
                    //Keywords: break
                    //Reserve Words: bool
                    //Noise Words: begin
                    buffer[idx++] = c;
                    state = 9;
                    break;
                } else if (c == 'r') {
                    //Keywords: return
                    buffer[idx++] = c;
                    state = 10;
                    break;
                } else if (c == 'o') {
                    //Keywords: output
                    //Logical: or
                    buffer[idx++] = c;
                    state = 11;
                    break;
                } else if (c == 'a') {
                    //Logical: and
                    buffer[idx++] = c;
                    state = 76;
                    break;
                } else if (c == 'c') {
                    //Keywords: const, continue
                    //Reserve Words: char
                    buffer[idx++] = c;
                    state = 12;
                    break;
                } else if (c == 'm') {
                    //Keywords: main
                    buffer[idx++] = c;
                    state = 13;
                    break;
                } else if (c == 's') {
                    //Reserve Words: str
                    buffer[idx++] = c;
                    state = 14;
                    break;
                } else if (c == 'v') {
                    //Reserve Words: void
                    buffer[idx++] = c;
                    state = 15;
                    break;
                } else if (c == 'n') {
                    //Reserve Words: null
                    //Logical: not
                    buffer[idx++] = c;
                    state = 16;
                    break;
                } else if (c == 't') {
                    //Noise Words: then
                    buffer[idx++] = c;
                    state = 17;
                    break;
                } else if (c == 'd') {
                    //Noise Words: do
                    buffer[idx++] = c;
                    state = 18;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }

            case 2:
                if (c == 'T') {
                    //Reserve Words: True
                    buffer[idx++] = c;
                    state = 19;
                    break;
                } else if (c == 'F') {
                    //Reserve Words: False
                    buffer[idx++] = c;
                    state = 20;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }

            case 3:
                //FOR NUMBERS
                if (isDigit(c) || c == '.') {
                    buffer[idx++] = c;
                } else {
                    // Check if followed by letter or underscore (invalid identifier)
                    int is_letter = 0;
                    for (int i = 0; i < lensm; i++) {
                        if (c == sm[i] || c == cap[i]) {
                            is_letter = 1;
                            break;
                        }
                    }

                    if (is_letter || c == '_') {
                        // ERROR: Identifier cannot start with a digit
                        buffer[idx++] = c;
                        state = 113; // Go to error state
                        break;
                    } else {
                        // Valid number termination (space, operator, semicolon, etc.)
                        buffer[idx] = '\0';
                        printf("NUMBER(%s)\n", buffer);
                        fprintf(out, "NUMBER(%s)\n", buffer);
                        idx = 0;
                        state = 0;
                        ungetc(c, fp);  // Put back the terminating character
                        break;
                    }
                }
                break;

            case 4:
                //FOR SYMBOLS, DELIMETERS AND BRACKETS
                if (c == '*') {
                    state = 116;
                    ungetc(c, fp);
                    break;
                } else if (c == '/') {
                    state = 117;
                    ungetc(c, fp);
                    break;
                }
                else if (c == '+') {
                    state = 118;
                    ungetc(c, fp);
                    break;
                }
                else if (c == '-') {
                    state = 119;
                    ungetc(c, fp);
                    break;
                }
                else if (c == '%') {
                    state = 120;
                    ungetc(c, fp);
                    break;
                }
                else if (c == '=') {
                    state = 121;
                    ungetc(c, fp);
                    break;
                }
                else if (c == '>') {
                    state = 123;
                    ungetc(c, fp);
                    break;
                }
                else if (c == '<') {
                    state = 124;
                    ungetc(c, fp);
                    break;
                }
                else if (c == '!') {
                    state = 122;
                    ungetc(c, fp);
                    break;
                }
                else if (c == '(') {
                    state = 125;
                    ungetc(c, fp);
                    break;
                }
                else if (c == ')') {
                    state = 126;
                    ungetc(c, fp);
                    break;

                }
                else if (c == '{') {
                    state = 127;
                    ungetc(c, fp);
                    break;

                }
                else if (c == '}') {
                    state = 128;
                    ungetc(c, fp);
                    break;

                }
                else if (c == '[') {
                    state = 129;
                    ungetc(c, fp);
                    break;
                }
                else if (c == ']') {
                    state = 130;
                    ungetc(c, fp);
                    break;
                }
                else if (c == ';') {
                    state = 131;
                    ungetc(c, fp);
                    break;
                }
                else if (c == '#') {
                    state = 132;
                    ungetc(c, fp);
                    break;
                } else if (c == ',') {
                    state = 142;
                    ungetc(c, fp);
                    break;
                }  else if (c == '"') {
                    idx = 0;
                    state = 114; // string state
                    break;
                }
                else if (c == '\'') {
                    idx = 0;
                    state = 115; // character state
                    break;
                }
                // For unrecognized symbols
                else {
                    buffer[idx++] = c;
                    state = 113;
                    break;
                }
            case 5:
                //Keywords: if, input,
                //Reserve Words: int
                if (c == 'f') {
                    //Papunta na sa final state si 'if' for printing
                    buffer[idx++] = c;
                    state = 50; // keyword
                    ungetc(c, fp);
                    break;
                } else if (c == 'n') {
                    buffer[idx++] = c;
                    state = 21;
                    break;
                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }

            case 6:
                //Keywords: else
                //Noise Words: end
                if (c == 'l') {
                    buffer[idx++] = c;
                    state = 22;
                    break;
                } else if (c == 'n') {
                    buffer[idx++] = c;
                    state = 23;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 7:
                //Keywords: for
                //Reserve Words: float
                if (c == 'o') {
                    buffer[idx++] = c;
                    state = 24;
                    break;
                } else if (c == 'l') {
                    buffer[idx++] = c;
                    state = 25;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }

            case 8:
                //Keyword: while
                if (c == 'h') {
                    buffer[idx++] = c;
                    state = 26;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }

            case 9:
                //Keywords: break
                //Reserve Words: bool
                //Noise Words: begin
                if (c == 'r') {
                    buffer[idx++] = c;
                    state = 27;
                    break;
                } else if (c == 'o') {
                    buffer[idx++] = c;
                    state = 28;
                    break;
                } else if (c == 'e') {
                    buffer[idx++] = c;
                    state = 29;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 10:
                //Keyword: return
                if (c == 'e') {
                    buffer[idx++] = c;
                    state = 30;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 11:
                //Keyword: output
                //Logical: or
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 31;
                    break;
                } else if (c == 'r') {
                    buffer[idx++] = c;
                    state = 74; // logical operator final state
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 12:
                //Keywords: const, continue
                //Reserve Words: char
                if (c == 'o') {
                    buffer[idx++] = c;
                    state = 32;
                    break;
                } else if (c == 'h') {
                    buffer[idx++] = c;
                    state = 33;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 13:
                //Keywords: main
                if (c == 'a') {
                    buffer[idx++] = c;
                    state = 34;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 14:
                //Reserve Words: str
                if (c == 't') {
                    buffer[idx++] = c;
                    state = 68;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 15:
                //Reserve Words: void
                if (c == 'o') {
                    buffer[idx++] = c;
                    state = 35;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 16:
                //Reserve Words: null
                //Logical: not
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 36;
                    break;
                } else if (c == 'o') {
                    buffer[idx++] = c;
                    state = 78;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 17:
                //Noise Words: then
                if (c == 'h') {
                    buffer[idx++] = c;
                    state = 37;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 18:
                //Noise Words: do
                if (c == 'o') {
                    buffer[idx++] = c;
                    state = 75; //Noise word
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 19:
                //Reserve Words: True
                if (c == 'r') {
                    buffer[idx++] = c;
                    state = 38;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 20:
                //Reserve Words: False
                if (c == 'a') {
                    buffer[idx++] = c;
                    state = 39;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 21:
                //Keywords: input,
                //Reserve Words: int
                if (c == 'p') {
                    buffer[idx++] = c;
                    state = 40;
                    break;
                } else if (c == 't') {
                    buffer[idx++] = c;
                    state = 100; // reserve word
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 22:
                //Keywords: else
                if (c == 's') {
                    buffer[idx++] = c;
                    state = 41;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 23:
                //Noise Words: end
                if (c == 'd') {
                    buffer[idx++] = c;
                    state = 75; //Noise word
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 24:
                //Keywords: for
                if (c == 'r') {
                    buffer[idx++] = c;
                    state = 50; // keyword
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 25:
                //Reserve Words: float
                if (c == 'o') {
                    buffer[idx++] = c;
                    state = 42;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 26:
                //Keyword: while
                if (c == 'i') {
                    buffer[idx++] = c;
                    state = 43;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 27:
                //Keywords: break
                if (c == 'e') {
                    buffer[idx++] = c;
                    state = 44;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 28:
                //Reserve Words: bool
                if (c == 'o') {
                    buffer[idx++] = c;
                    state = 45;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 29:
                //Noise Words: begin
                if (c == 'g') {
                    buffer[idx++] = c;
                    state = 46;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 30:
                //Keyword: return
                if (c == 't') {
                    buffer[idx++] = c;
                    state = 47;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 31:
                //Keyword: output
                if (c == 't') {
                    buffer[idx++] = c;
                    state = 48;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 32:
                //Keywords: const, continue
                if (c == 'n') {
                    buffer[idx++] = c;
                    state = 49;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 33:
                //Reserve Words: char
                if (c == 'a') {
                    buffer[idx++] = c;
                    state = 51;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 34:
                //Keywords: main
                if (c == 'i') {
                    buffer[idx++] = c;
                    state = 52;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 35:
                //Reserve Words: void
                if (c == 'i') {
                    buffer[idx++] = c;
                    state = 53;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 36:
                //Reserve Words: null
                if (c == 'l') {
                    buffer[idx++] = c;
                    state = 54;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 37:
                //Noise Words: then
                if (c == 'e') {
                    buffer[idx++] = c;
                    state = 55;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 38:
                //Reserve Words: True
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 56;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 39:
                //Reserve Words: False
                if (c == 'l') {
                    buffer[idx++] = c;
                    state = 57;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 40:
                //Keywords: input,
                //Reserve Words: int
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 58;
                    break;
                } else if (c == 't') {
                    buffer[idx++] = c;
                    state = 100; // reserve word
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }

            case 41:
                //Keywords: else
                if (c == 'e') {
                    buffer[idx++] = c;
                    state = 50;
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 42:
                //Reserve Words: float
                if (c == 'a') {
                    buffer[idx++] = c;
                    state = 59;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 43:
                //Keyword: while
                if (c == 'l') {
                    buffer[idx++] = c;
                    state = 60;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 44:
                //Keywords: break
                if (c == 'a') {
                    buffer[idx++] = c;
                    state = 61;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 45:
                //Reserve Words: bool
                if (c == 'l') {
                    buffer[idx++] = c;
                    state = 100; // reserve word
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 46:
                //Noise Words: begin
                if (c == 'i') {
                    buffer[idx++] = c;
                    state = 62;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 47:
                //Keyword: return
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 63;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 48:
                //Keyword: output
                if (c == 'p') {
                    buffer[idx++] = c;
                    state = 64;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 49:
                //Keywords: const, continue
                if (c == 's') {
                    buffer[idx++] = c;
                    state = 65;
                    break;
                } else if (c == 't') {
                    buffer[idx++] = c;
                    state = 66;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 50:
                int next = fgetc(fp);
                if (next == ' ' || next == '\n' || next == '\t' || next == '(' || next == ';' || next == '{' || next == '}' || next == ':' || next == ')' || next == -1) {
                    buffer[idx] = '\0';
                    printf("KEYWORD(%s)\n", buffer);
                    fprintf(out, "KEYWORD(%s)\n", buffer);
                    idx = 0;
                    state = 0;
                    exit = 0;
                    ungetc(next, fp);
                    break;
                } else {
                    // Don't put back c, just continue reading
                    buffer[idx++] = next;  // Add the next character instead
                    state = 110; // identifier
                    break;
                }
            case 51:
                //Reserve Words: char
                if (c == 'r') {
                    buffer[idx++] = c;
                    state = 100; // reserve word
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 52:
                //Keywords: main
                if (c == 'n') {
                    buffer[idx++] = c;
                    state = 50;
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 53:
                //Reserve Words: void
                if (c == 'd') {
                    buffer[idx++] = c;
                    state = 100;
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 54:
                //Reserve Words: null
                if (c == 'l') {
                    buffer[idx++] = c;
                    state = 100;
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 55:
                //Noise Words: then
                if (c == 'n') {
                    buffer[idx++] = c;
                    state = 75;
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 56:
                //Reserve Words: True
                if (c == 'e') {
                    buffer[idx++] = c;
                    state = 100;
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 57:
                //Reserve Words: False
                if (c == 's') {
                    buffer[idx++] = c;
                    state = 67;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 58:
                //Keywords: input
                if (c == 't') {
                    buffer[idx++] = c;
                    state = 50;
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 59:
                //Reserve Words: float
                if (c == 't') {
                    buffer[idx++] = c;
                    state = 100;
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 60:
                //Keyword: while
                if (c == 'e') {
                    buffer[idx++] = c;
                    state = 50; // keyword
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 61:
                //Keywords: break
                if (c == 'k') {
                    buffer[idx++] = c;
                    state = 50; // keyword
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 62:
                //Noise Words: begin
                if (c == 'n') {
                    buffer[idx++] = c;
                    state = 75;
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 63:
                //Keyword: return
                if (c == 'r') {
                    buffer[idx++] = c;
                    state = 69;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 64:
                //Keyword: output
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 70;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 65:
                //Keywords: continue
                if (c == 't') {
                    buffer[idx++] = c;
                    state = 50; // keyword
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 66:
                //Keywords: continue
                if (c == 'i') {
                    buffer[idx++] = c;
                    state = 71;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 67:
                //Reserve Words: False
                if (c == 'e') {
                    buffer[idx++] = c;
                    state = 100;
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 68:
                //Reserve word: str
                if (c == 'r') {
                    buffer[idx++] = c;
                    state = 100;
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 69:
                //Keyword: return
                if (c == 'n') {
                    buffer[idx++] = c;
                    state = 50; // keyword
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 70:
                //Keyword: output
                if (c == 't') {
                    buffer[idx++] = c;
                    state = 50; // keyword
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 71:
                //Keywords: continue
                if (c == 'n') {
                    buffer[idx++] = c;
                    state = 72;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }

            case 72:
                //Keywords: continue
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 73;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 73:
                //Keywords: continue
                if (c == 'e') {
                    buffer[idx++] = c;
                    state = 50; // keyword
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }
            case 74:
                // Final state for logical operators
                next = fgetc(fp);
                if (next == ' ' || next == '\n' || next == '\t' || next == '(' || next == ';' || next == '{' || next == '}' || next == ':' || next == ')' || next == -1) {
                    buffer[idx] = '\0';
                    printf("LOGICAL(%s)\n", buffer);
                    fprintf(out, "LOGICAL(%s)\n", buffer);
                    idx = 0;
                    state = 0;
                    exit = 0;
                    ungetc(next, fp);
                    break;
                } else {
                    buffer[idx++] = next;  // Add the next character instead
                    state = 110; // identifier
                    break;
                }
            case 75:
                next = fgetc(fp);
                if (next == ' ' || next == '\n' || next == '\t' || next == '(' || next == ';' || next == '{' || next == '}' || next == ':' || next == ')' || next == -1) {
                    buffer[idx] = '\0';
                    printf("NOISE_WORD(%s)\n", buffer);
                    fprintf(out, "NOISE_WORD(%s)\n", buffer);
                    idx = 0;
                    state = 0;
                    exit = 0;
                    ungetc(next, fp);
                    break;
                } else {
                    buffer[idx++] = next;  // Add the next character instead
                    state = 110; // identifier
                    break;
                }
            case 76:
                //Logical: and
                if (c == 'n') {
                    buffer[idx++] = c;
                    state = 79;
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }

            case 78:
                //Logical: not
                if (c == 't') {
                    buffer[idx++] = c;
                    state = 74; // logical operator final state
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }

            case 79:
                //Logical: and
                if (c == 'd') {
                    buffer[idx++] = c;
                    state = 74; // logical operator final state
                    ungetc(c, fp);
                    break;
                                } else {
                    state = 110;
                    ungetc(c, fp); // identifier
                    break;
                }

            case 100:
                next = fgetc(fp);
                if (next == ' ' || next == '\n' || next == '\t' || next == '(' || next == ';' || next == '{' || next == '}' || next == ':' || next == ')' || next == -1) {
                    buffer[idx] = '\0';
                    printf("RESERVED_WORD(%s)\n", buffer);
                    fprintf(out, "RESERVED_WORD(%s)\n", buffer);
                    idx = 0;
                    state = 0;
                    exit = 0;
                    ungetc(next, fp);
                    break;
                } else {
                    buffer[idx++] = next;  // Add the next character instead
                    state = 110; // identifier
                    break;
                }

            case 110:
                {
                    int is_letter = 0;
                    int is_digit = 0;
                    int is_underscore = 0;

                    if (isLetter(c, sm) || isLetter(c, cap)) {
                        is_letter = 1;
                    }

                    if (isDigit(c)) {
                        is_digit = 1;
                    }

                    if (c == '_') {
                        is_underscore = 1;
                    }

                    // Only valid identifier characters (letters, digits, underscore) can continue
                    if (is_letter || is_digit || is_underscore) {
                        buffer[idx++] = c;
                        state = 110; // Stay in identifier state
                        break;
                    } else {
                        // Any other character (space, operator, bracket, colon, etc.) terminates the identifier
                        buffer[idx] = '\0';
                        printf("IDENTIFIER(%s)\n", buffer);
                        fprintf(out, "IDENTIFIER(%s)\n", buffer);
                        idx = 0;
                        state = 0;
                        ungetc(c, fp);  // Put back the terminating character
                        break;
                    }
                }

            case 111:
                // Single-line comment state
                if (c == '\n') {
                    buffer[idx] = '\0';
                    if (idx > 0) {
                        printf("COM_STR(%s)\n", buffer);
                        fprintf(out, "COM_STR(%s)\n", buffer);
                    }
                    printf("NEW_LINE(\\n)\n");
                    fprintf(out, "NEW_LINE(\\n)\n");
                    idx = 0;
                    state = 0;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 111;
                    break;
                }

            case 112:
                // Multi-line comment state
                if (c == '*') {
                    int next = fgetc(fp);
                    if (next == '/') {
                        buffer[idx] = '\0';
                        if (idx > 0) {
                            printf("COM_STR(%s)\n", buffer);
                            fprintf(out, "COM_STR(%s)\n", buffer);
                        }
                        printf("COMMENT_END(*/)\n");
                        fprintf(out, "COMMENT_END(*/)\n");
                        idx = 0;
                        state = 0;
                        break;
                    } else {
                        buffer[idx++] = c;
                        ungetc(next, fp);
                        state = 112;
                        break;
                    }
                } else {
                    buffer[idx++] = c;
                    state = 112;
                    break;
                }

            case 113:
                // Error state
                {
                    int is_valid = 0;


                        if (isLetter(c, sm) || isLetter(c, cap)) {
                            is_valid = 1;
                        }
                        if (isDigit(c) && !is_valid) {
                            is_valid = 1;
                        }
                    if (c == '_') is_valid = 1;

                    if (is_valid) {
                        buffer[idx++] = c;
                        state = 113;
                        break;
                    } else {
                        buffer[idx] = '\0';
                        printf("ERROR(%s)\n", buffer);
                        fprintf(out, "ERROR(%s)\n", buffer);
                        idx = 0;
                        state = 0;
                        ungetc(c, fp);
                        break;
                    }
                }

            case 114:
                if (c == '"') {
                    buffer[idx] = '\0';

                    // Check if the string contains interpolation
                    if (strchr(buffer, '{') != NULL && strchr(buffer, '}') != NULL) {
                        printf("STRING_INTERP(%s)\n", buffer);
                        fprintf(out, "STRING_INTERP(%s)\n", buffer);
                    } else {
                        printf("STRING(%s)\n", buffer);
                        fprintf(out, "STRING(%s)\n", buffer);
                    }

                    idx = 0;
                    state = 0;
                    break;
                }
                else if (c == '\n') {
                    buffer[idx] = '\0';
                    printf("ERROR(Unterminated string: %s)\n", buffer);
                    fprintf(out, "ERROR(Unterminated string: %s)\n", buffer);
                    idx = 0;
                    state = 0;
                    break;
                }
                else {
                    buffer[idx++] = c;
                    state = 114;
                    break;
                }


            case 115:
                // Character literal state
                if (c == '\'') {
                    buffer[idx] = '\0';
                    if (idx == 1) {
                        printf("CHAR(%s)\n", buffer);
                        fprintf(out, "CHAR(%s)\n", buffer);
                    } else {
                        printf("ERROR(Invalid character literal: %s)\n", buffer);
                        fprintf(out, "ERROR(Invalid character literal: %s)\n", buffer);
                    }
                    idx = 0;
                    state = 0;
                    break;
                } else if (c == '\n') {
                    buffer[idx] = '\0';
                    printf("ERROR(Unterminated character: %s)\n", buffer);
                    fprintf(out, "ERROR(Unterminated character: %s)\n", buffer);
                    idx = 0;
                    state = 0;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 115;
                    break;
                }
            case 116:
                if (c == '*') {
                    int next = fgetc(fp);
                    if (next == '*') {
                        state = 138;
                        ungetc(next, fp);
                        break;
                    } else {
                        printf("ARITH_OP(*)\n");
                        fprintf(out, "ARITH_OP(*)\n");
                        state = 0;
                        ungetc(next, fp);
                        break;
                    }
                }
            case 117:
                if (c == '/') {
                    int next = fgetc(fp);
                    if (next == '/') {
                        // Floor division operator
                        state = 133;
                        ungetc(next, fp);
                        break;

                    } else if (next == '*') {
                        state = 134;
                        ungetc(next, fp);
                        break;
                    } else {
                        printf("ARITH_OP(/)\n");
                        fprintf(out, "ARITH_OP(/)\n");
                        ungetc(next, fp);
                        state = 0;
                        break;
                    }
                }

            case 118:
                if (c == '+') {
                    int next = fgetc(fp);
                    if (next == '+') {
                        state = 136;
                        ungetc(next, fp);
                        break;
                    } else {
                        printf("ARITH_OP(+)\n");
                        fprintf(out, "ARITH_OP(+)\n");
                        ungetc(next, fp);
                    }
                    state = 0;
                    break;
                }
            case 119:
                if (c == '-') {
                    int next = fgetc(fp);
                    if (next == '-') {
                        state = 137;
                        ungetc(next, fp);
                        break;
                    } else {
                        printf("ARITH_OP(-)\n");
                        fprintf(out, "ARITH_OP(-)\n");
                        ungetc(next, fp);
                    }
                    state = 0;
                    break;
                }
            case 120:
                if (c == '%') {
                    printf("ARITH_OP(%%)\n");
                    fprintf(out, "ARITH_OP(%%)\n");
                    state = 0;
                    break;
                }
            case 121:
                if (c == '=') {
                    int next = fgetc(fp);
                    if (next == '=') {
                        ungetc(next, fp);
                        state = 135;
                        break;
                    } else {
                        // Assignment operator
                        printf("ASSIGN_OP(=)\n");
                        fprintf(out, "ASSIGN_OP(=)\n");
                        ungetc(next, fp);
                    }
                    state = 0;
                    break;
                }
            case 122:
                if (c == '!') {
                    int next = fgetc(fp);
                    if (next == '=') {
                        state = 139;
                        ungetc(next, fp);
                        break;
                    } else {
                        printf("ERROR(!)\n");
                        fprintf(out, "ERROR(!)\n");
                        ungetc(next, fp);
                    }
                    state = 0;
                    break;
                }
            case 123:
                if (c == '>') {
                    int next = fgetc(fp);
                    if (next == '=') {
                        state = 140;
                        ungetc(next, fp);
                        break;
                    } else {
                        printf("REL_OP(>)\n");
                        fprintf(out, "REL_OP(>)\n");
                        ungetc(next, fp);
                    }
                    state = 0;
                    break;
                }
            case 124:
                if (c == '<') {
                    int next = fgetc(fp);
                    if (next == '=') {
                        state = 141;
                        ungetc(next, fp);
                        break;
                    } else {
                        printf("REL_OP(<)\n");
                        fprintf(out, "REL_OP(<)\n");
                        ungetc(next, fp);
                    }
                    state = 0;
                    break;
                }
            case 125:
                if (c == '(') {
                    printf("LEFT_PAREN(()\n");
                    fprintf(out, "LEFT_PAREN(()\n");
                    state = 0;
                    break;
                }
            case 126:
                if (c == ')') {
                    printf("RIGHT_PAREN())\n");
                    fprintf(out, "RIGHT_PAREN())\n");
                    state = 0;
                    break;
                }
            case 127:
                if (c == '{') {
                    printf("LEFT_BRACE({)\n");
                    fprintf(out, "LEFT_BRACE({)\n");
                    state = 0;
                    break;
                }
            case 128:
                if (c == '}') {
                    printf("RIGHT_BRACE(})\n");
                    fprintf(out, "RIGHT_BRACE(})\n");
                    state = 0;
                    break;
                }
            case 129:
                if (c == '[') {
                    printf("LEFT_BRACKET([)\n");
                    fprintf(out, "LEFT_BRACKET([)\n");
                    state = 0;
                    break;
                }
            case 130:
                if (c == ']') {
                    printf("RIGHT_BRACKET(])\n");
                    fprintf(out, "RIGHT_BRACKET(])\n");
                    state = 0;
                    break;
                }
            case 131:
                if (c == ';') {
                    printf("SEMICOLON(;)\n");
                    fprintf(out, "SEMICOLON(;)\n");
                    state = 0;
                    break;
                }
            case 132:
                if (c == '#') {
                    printf("COMMENT(#)\n");
                    fprintf(out, "COMMENT(#)\n");
                    idx = 0;
                    state = 111; // single-line comment state
                    break;
                }
            case 133:
                if (c == '/') {
                    printf("ARITH_OP(//)\n");
                    fprintf(out, "ARITH_OP(//)\n");
                    state = 0;
                    break;
                }
            case 134:
                if (c == '*') {
                    // Multi-line comment start
                    printf("COMMENT_START(/*)\n");
                    fprintf(out, "COMMENT_START(/*)\n");
                    idx = 0;
                    state = 112; // multi-line comment state
                    break;
                }
            case 135:
                if (c == '=') {
                    printf("REL_OP(==)\n");
                    fprintf(out, "REL_OP(==)\n");
                    state = 0;
                    break;
                }
            case 136:
                if (c == '+') {
                    printf("INCREMENT(++)\n");
                    fprintf(out, "INCREMENT(++)\n");
                    state = 0;
                    break;
                }
            case 137:
                if (c == '-') {
                    printf("DECREMENT(--)\n");
                    fprintf(out, "DECREMENT(--)\n");
                    state = 0;
                    break;
                }
            case 138:
                if (c == '*') {
                    printf("ARITH_OP(**)\n");
                    fprintf(out, "ARITH_OP(**)\n");
                    state = 0;
                    break;
                }
            case 139:
                if (c == '=') {
                    printf("REL_OP(!=)\n");
                    fprintf(out, "REL_OP(!=)\n");
                    state = 0;
                    break;
                }
            case 140:
                if (c == '=') {
                    printf("REL_OP(>=)\n");
                    fprintf(out, "REL_OP(>=)\n");
                    state = 0;
                    break;
                }
            case 141:
                if (c == '=') {
                    printf("REL_OP(<=)\n");
                    fprintf(out, "REL_OP(<=)\n");
                    state = 0;
                    break;
                }
            case 142:
                if (c == ',') {
                    printf("COMMA(,)\n");
                    fprintf(out, "COMMA(,)\n");
                    state = 0;
                    break;
                }
            default:
                break;
        }
    }

if (idx > 0) {
    buffer[idx] = '\0';
    if (state == 110) {
        printf("IDENTIFIER(%s)\n", buffer);
        fprintf(out, "IDENTIFIER(%s)\n", buffer);
    } else if (state == 50) {
        printf("KEYWORD(%s)\n", buffer);
        fprintf(out, "KEYWORD(%s)\n", buffer);
    } else if (state == 100) {
        printf("RESERVED_WORD(%s)\n", buffer);
        fprintf(out, "RESERVED_WORD(%s)\n", buffer);
    } else if (state == 75) {
        printf("NOISE_WORD(%s)\n", buffer);
        fprintf(out, "NOISE_WORD(%s)\n", buffer);
    } else if (state == 74) {
        printf("LOGICAL(%s)\n", buffer);
        fprintf(out, "LOGICAL(%s)\n", buffer);
    } else if (state == 3) {
        printf("NUMBER(%s)\n", buffer);
        fprintf(out, "NUMBER(%s)\n", buffer);
    } else if (state == 111) {
        printf("COM_STR(%s)\n", buffer);
        fprintf(out, "COM_STR(%s)\n", buffer);
    } else if (state == 113) {
        printf("ERROR(%s)\n", buffer);
        fprintf(out, "ERROR(%s)\n", buffer);
    }
}

    fclose(fp);
    fclose(out);
    return 0;
}