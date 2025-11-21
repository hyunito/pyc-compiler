#include <stdio.h>


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

int main() {
    char filename[] = "sample.pyclang";

    char cap[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
        'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
    };

    char sm[] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
    };

    char symbols[] = {
        '`', '~', '@', '!', '$', '#', '^', '*', '%', '&', '(', ')', '[', ']', '{', '}', '<', '>',
        '+', '=', '_', '-', '|', '/', '\\', ';', ':', '\'', '"', ',', '.'
    };

    char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

    int lines = 1;

    int lensm = sizeof(sm) / sizeof(sm[0]);
    int lensymbols = sizeof(symbols) / sizeof(symbols[0]);
    int lendigits = sizeof(digits) / sizeof(digits[0]);

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
        //printf("state = %d, character = %c, its num = %d\n", state, c, c);
        switch (state) {

            //Starting state 0
            case 0:

                for (int i = 0; i < lensm; i++) {
                    //Go to State 1 if small letters
                    if (c == sm[i]) {
                        state = 1;
                        ungetc(c, fp);
                        exit = 1;
                        break;
                    }
                    //Go to State 2 if capital letters
                    else if (c == cap[i]) {
                        state = 2;
                        ungetc(c, fp);
                        exit = 1;
                        break;
                    }
                }


                for (int i = 0; i < lendigits; i++) {
                    //Go to State 3 if digits
                    if (c == digits[i]) {
                        state = 3;
                        ungetc(c, fp);
                        exit = 1;
                        break;
                    }
                }

                for (int i = 0; i < lensymbols; i++) {
                    //Go to State 4 if symbols
                    if (c == symbols[i]) {
                        state = 4;
                        ungetc(c, fp);
                        exit = 1;
                        break;
                    }
                }
                if (exit == 1) {
                    break;
                }
                if (c == ' ' || c == '\t' || c == -1) {
                    break;
                } else if (c == '\n') {
                    printf("NEW_LINE(\\n)\n");
                    fprintf(out, "NEW_LINE(\\n)\n", c);
                    break;
                } else {
                    printf("The character or symbol \"%c\" is not yet registered.\n", c);
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
                    buffer[idx++] = c;
                    state = 11;
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }

            case 3:
                //FOR DIGITS
                break;

            case 4:
                //FOR SYMBOLS
                break;

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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }

            case 8:
                //Keyword: while
                if (c == 'h') {
                    buffer[idx++] = c;
                    state = 26;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 10:
                //Keyword: return
                if (c == 'e') {
                    buffer[idx++] = c;
                    state = 30;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 11:
                //Keyword: output
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 31;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 13:
                //Keywords: main
                if (c == 'a') {
                    buffer[idx++] = c;
                    state = 34;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 14:
                //Reserve Words: str
                if (c == 't') {
                    buffer[idx++] = c;
                    state = 68;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 15:
                //Reserve Words: void
                if (c == 'o') {
                    buffer[idx++] = c;
                    state = 35;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 16:
                //Reserve Words: null
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 36;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 17:
                //Noise Words: then
                if (c == 'h') {
                    buffer[idx++] = c;
                    state = 37;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 19:
                //Reserve Words: True
                if (c == 'r') {
                    buffer[idx++] = c;
                    state = 38;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 20:
                //Reserve Words: False
                if (c == 'a') {
                    buffer[idx++] = c;
                    state = 39;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 22:
                //Keywords: else
                if (c == 's') {
                    buffer[idx++] = c;
                    state = 41;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 25:
                //Reserve Words: float
                if (c == 'o') {
                    buffer[idx++] = c;
                    state = 42;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 26:
                //Keyword: while
                if (c == 'i') {
                    buffer[idx++] = c;
                    state = 43;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 27:
                //Keywords: break
                if (c == 'e') {
                    buffer[idx++] = c;
                    state = 44;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 28:
                //Reserve Words: bool
                if (c == 'o') {
                    buffer[idx++] = c;
                    state = 45;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 29:
                //Noise Words: begin
                if (c == 'g') {
                    buffer[idx++] = c;
                    state = 46;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 30:
                //Keyword: return
                if (c == 't') {
                    buffer[idx++] = c;
                    state = 47;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 31:
                //Keyword: output
                if (c == 't') {
                    buffer[idx++] = c;
                    state = 48;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 32:
                //Keywords: const, continue
                if (c == 'n') {
                    buffer[idx++] = c;
                    state = 49;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 33:
                //Reserve Words: char
                if (c == 'a') {
                    buffer[idx++] = c;
                    state = 51;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 34:
                //Keywords: main
                if (c == 'i') {
                    buffer[idx++] = c;
                    state = 52;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 35:
                //Reserve Words: void
                if (c == 'i') {
                    buffer[idx++] = c;
                    state = 53;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 36:
                //Reserve Words: null
                if (c == 'l') {
                    buffer[idx++] = c;
                    state = 54;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 37:
                //Noise Words: then
                if (c == 'e') {
                    buffer[idx++] = c;
                    state = 55;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 38:
                //Reserve Words: True
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 56;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 39:
                //Reserve Words: False
                if (c == 'l') {
                    buffer[idx++] = c;
                    state = 57;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 42:
                //Reserve Words: float
                if (c == 'a') {
                    buffer[idx++] = c;
                    state = 59;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 43:
                //Keyword: while
                if (c == 'l') {
                    buffer[idx++] = c;
                    state = 60;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 44:
                //Keywords: break
                if (c == 'a') {
                    buffer[idx++] = c;
                    state = 61;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 46:
                //Noise Words: begin
                if (c == 'i') {
                    buffer[idx++] = c;
                    state = 62;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 47:
                //Keyword: return
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 63;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 48:
                //Keyword: output
                if (c == 'p') {
                    buffer[idx++] = c;
                    state = 64;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 50:
                int next = fgetc(fp);
                //printf("case50: next = \"%c\"\n", next);
                if (next == ' ' || next == '\n' || next == '\t' || next == '(' || next == ';' || next == '{' || next == '}' || next == -1) {
                    buffer[idx] = '\0';
                    printf("KEYWORD(%s)\n", buffer);
                    fprintf(out, "KEYWORD(%s)\n", buffer);
                    idx = 0;
                    state = 0;
                    exit = 0;
                    ungetc(next, fp);
                    break;
                } else {

                    buffer[idx++] = c;
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 57:
                //Reserve Words: False
                if (c == 's') {
                    buffer[idx++] = c;
                    state = 67;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 63:
                //Keyword: return
                if (c == 'r') {
                    buffer[idx++] = c;
                    state = 69;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 64:
                //Keyword: output
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 70;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 66:
                //Keywords: continue
                if (c == 'i') {
                    buffer[idx++] = c;
                    state = 71;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 71:
                //Keywords: continue
                if (c == 'n') {
                    buffer[idx++] = c;
                    state = 72;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }

            case 72:
                //Keywords: continue
                if (c == 'u') {
                    buffer[idx++] = c;
                    state = 73;
                    break;
                } else {
                    buffer[idx++] = c;
                    state = 110; // identifier
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
                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 75:
                next = fgetc(fp);
                //printf("case75: next = \"%c\"\n", next);
                if (next == ' ' || next == '\n' || next == '\t' || next == '(' || next == ';' || next == '{' || next == '}' || next == -1) {
                    buffer[idx] = '\0';
                    printf("NOISE_WORD(%s)\n", buffer);
                    fprintf(out, "NOISE_WORD(%s)\n", buffer);
                    idx = 0;
                    state = 0;
                    exit = 0;
                    ungetc(next, fp);
                    break;
                } else {

                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }
            case 100:
                next = fgetc(fp);
                //printf("case75: next = \"%c\"\n", next);
                if (next == ' ' || next == '\n' || next == '\t' || next == '(' || next == ';' || next == '{' || next == '}' || next == -1) {
                    buffer[idx] = '\0';
                    printf("RESERVED_WORD(%s)\n", buffer);
                    fprintf(out, "RESERVED_WORD(%s)\n", buffer);
                    idx = 0;
                    state = 0;
                    exit = 0;
                    ungetc(next, fp);
                    break;
                } else {

                    buffer[idx++] = c;
                    state = 110; // identifier
                    break;
                }

            default:
                break;
        }
    }

    fclose(fp);
    fclose(out);
    return 0;
}