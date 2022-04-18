#include "compiler.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUMBER_TOKENS 1000
#define MAX_IDENT_LEN 11
#define MAX_NUMBER_LEN 5

lexeme *list;
int lex_index;
int input_index;

int numbertoken();

int symboltoken();

void comment();

void printlexerror(int type);

void printtokens();

int getWord(char *input);

// Returns the number in the input string or -1 if the number is too large
int getInt(char *input) {
    int pos = input_index;
    char *str;

    while (isdigit(input[input_index])) {
        input_index++;
    }

    str = (char *) malloc(sizeof(char) * (input_index - pos + 1));

    if (input_index - pos <= MAX_NUMBER_LEN) {
        for (int i = 0; i < input_index - pos; i++) {
            str[i] = input[pos + i];
        }

        if (isalpha(input[input_index + 1])) {
            return -1;
        }

        return atoi(str);
    }

    // number is more than 5 digits
    return -2;
}

int getWord(char *input) {

    int length = 0;
    int pos = input_index; // original input_index is saved s owe know where to
    // start after determininng the length of the word

    if (input[input_index] == ' ') {
        while (input[input_index] == ' ') {
            input_index++;
        }
    }

    while (isalpha(input[input_index])) {
        length++;
        input_index++;
    }

    if (length > MAX_IDENT_LEN) {
        return 1;
    }

    char *word = (char *) malloc(sizeof(char) * (length + 1));

    for (int i = 0; i < length; i++) {
        word[i] = input[pos + i];
    }

    if (!word) {
        return 2;
    }

    if (strcmp(word, "var") == 0) {
        list[lex_index].type = varsym;
        list[lex_index].value = 2;
        strcpy(list[lex_index].name, "var");

    } else if (strcmp(word, "procedure") == 0) {
        list[lex_index].type = procsym;
        list[lex_index].value = 4;
        strcpy(list[lex_index].name, "procedure");

    } else if (strcmp(word, "call") == 0) {
        list[lex_index].type = callsym;
        list[lex_index].value = 6;
        strcpy(list[lex_index].name, "call");

    } else if (strcmp(word, "begin") == 0) {
        list[lex_index].type = beginsym;
        list[lex_index].value = 8;
        strcpy(list[lex_index].name, "begin");

    } else if (strcmp(word, "end") == 0) {
        list[lex_index].type = endsym;
        list[lex_index].value = 10;
        strcpy(list[lex_index].name, "end");

    } else if (strcmp(word, "if") == 0) {
        list[lex_index].type = ifsym;
        list[lex_index].value = 12;
        strcpy(list[lex_index].name, "if");

    } else if (strcmp(word, "do") == 0) {
        list[lex_index].type = dosym;
        list[lex_index].value = 14;
        strcpy(list[lex_index].name, "do");

    } else if (strcmp(word, "read") == 0) {
        list[lex_index].type = readsym;
        list[lex_index].value = 16;
        strcpy(list[lex_index].name, "read");

    } else if (strcmp(word, "write") == 0) {
        list[lex_index].type = writesym;
        list[lex_index].value = 18;
        strcpy(list[lex_index].name, "write");

    } else if (strcmp(word, "while") == 0) {
        list[lex_index].type = whilesym;
        list[lex_index].value = 33;
        strcpy(list[lex_index].name, "while");

    } else {
        list[lex_index].type = identsym;
        list[lex_index].value = 20;
        strcpy(list[lex_index].name, word);
    }

    return 0;
}

// Advances input_index until the comment is over
void comment(char *input) {
    while (!iscntrl(input[input_index])) {
        input_index++;
    }
}

lexeme *lexanalyzer(char *input, int printFlag) {
    list = (lexeme *) malloc(sizeof(lexeme) * MAX_NUMBER_TOKENS);
    lex_index = 0;
    input_index = 0;
    int flag = 0;

    while (input[input_index] != '\0') {
        TOP:

        if (flag) {
            return NULL;
        }

        int current = input[input_index];

        // Repeats loop until there isnt a \t, /n, or whitespace at current
        // index
        if (iscntrl(current) || isspace(current)) {
            input_index++;
            continue;
        } else if (isdigit(current)) {
            int val = getInt(input);

            if (val < 0) {
                flag = 1;
                if(val == -1 ) {
                    printlexerror(1);
                } else if (val == -2) {
                    printlexerror(2);
                }
                goto TOP;
            }

            list[lex_index].type = numbersym;
            list[lex_index].value = val;
            lex_index++;
            continue;
        } else if (isalpha(current)) {
            int error = getWord(input);
            if (error) {
                // TODO
                flag = 1;
                if (error == 1) {
                    printlexerror(3);
                } else if (error == 2) {
                    printlexerror(1);
                } else if (error == 3) {
                }
            }
            lex_index++;
            goto TOP;
        } else {
            switch (current) {
                case '.':
                    list[lex_index].type = periodsym;
                    list[lex_index].value = 1;
                    strcpy(list[lex_index].name, ".");
                    break;

                case '[':
                    list[lex_index].type = lbracketsym;
                    list[lex_index].value = 3;
                    strcpy(list[lex_index].name, "[");
                    break;

                case ']':
                    list[lex_index].type = rbracketsym;
                    list[lex_index].value = 5;
                    strcpy(list[lex_index].name, "]");
                    break;

                case ',':
                    list[lex_index].type = commasym;
                    list[lex_index].value = 7;
                    strcpy(list[lex_index].name, ",");
                    break;

                case ';':
                    list[lex_index].type = semicolonsym;
                    list[lex_index].value = 9;
                    strcpy(list[lex_index].name, ";");
                    break;

                case ':':
                    if (input[input_index + 1] == '=') {
                        list[lex_index].type = assignsym;
                        list[lex_index].value = 11;
                        strcpy(list[lex_index].name, ":=");
                        input_index++;
                    } else {
                        list[lex_index].type = colonsym;
                        list[lex_index].value = 15;
                        strcpy(list[lex_index].name, ":");
                    }
                    break;

                case '?':
                    list[lex_index].type = questionsym;
                    list[lex_index].value = 13;
                    strcpy(list[lex_index].name, "?");
                    break;

                case '(':
                    list[lex_index].type = lparenthesissym;
                    list[lex_index].value = 17;
                    strcpy(list[lex_index].name, "(");
                    break;

                case ')':
                    list[lex_index].type = rparenthesissym;
                    list[lex_index].value = 19;
                    strcpy(list[lex_index].name, ")");
                    break;

                case '=':
                    if (input[input_index + 1] == '=') {
                        list[lex_index].type = eqlsym;
                        list[lex_index].value = 21; // twahntee one
                        strcpy(list[lex_index].name, "==");
                        input_index++;
                    } else {
                        // TODO ERROR HANDLING
                    }
                    break;

                case '<':
                    if (input[input_index + 1] == '=') {
                        list[lex_index].type = leqsym;
                        list[lex_index].value = 27;
                        strcpy(list[lex_index].name, "<=");
                        input_index++;
                    } else if (input[input_index + 1] == '>') {
                        list[lex_index].type = neqsym;
                        list[lex_index].value = 23;
                        strcpy(list[lex_index].name, "<>");
                        input_index++;
                    } else {
                        list[lex_index].type = lsssym;
                        list[lex_index].value = 25;
                        strcpy(list[lex_index].name, "<");
                    }
                    break;

                case '>':
                    if (input[input_index + 1] == '=') {
                        list[lex_index].type = geqsym;
                        list[lex_index].value = 31;
                        strcpy(list[lex_index].name, ">=");
                        input_index++;
                    } else {
                        list[lex_index].type = gtrsym;
                        list[lex_index].value = 29;
                        strcpy(list[lex_index].name, ">");
                    }
                    break;

                case '+':
                    list[lex_index].type = addsym;
                    list[lex_index].value = 32;
                    strcpy(list[lex_index].name, "+");
                    break;

                case '-':
                    list[lex_index].type = subsym;
                    list[lex_index].value = 30;
                    strcpy(list[lex_index].name, "-");
                    break;

                case '*':
                    list[lex_index].type = multsym;
                    list[lex_index].value = 28;
                    strcpy(list[lex_index].name, "*");
                    break;

                case '/':
                    // If it is a comment then skip to the end of the line or
                    // end of the file
                    if (input[input_index + 1] == '/') {
                        comment(input);
                        goto TOP;
                    } else {
                        list[lex_index].type = divsym;
                        list[lex_index].value = 26;
                        strcpy(list[lex_index].name, "/");
                    }
                    break;

                case '%':
                    list[lex_index].type = modsym;
                    list[lex_index].value = 24;
                    strcpy(list[lex_index].name, "%");
                    break;

                case ' ':
                    while (input[input_index] == ' ') {
                        input_index++;
                    }
                    break;

                default:
                    printlexerror(4);
                    flag = 1;
                    break;
            }
        }

        lex_index++;
        input_index++;
    }

    printtokens();
    return list;
}

void printtokens() {
    int i;
    printf("Lexeme Table:\n");
    printf("lexeme\t\ttoken type\n");
    for (i = 0; i < lex_index; i++) {
        switch (list[i].type) {
            case periodsym:
                printf("%11s\t%d", ".", periodsym);
                break;
            case varsym:
                printf("%11s\t%d", "var", varsym);
                break;
            case lbracketsym:
                printf("%11s\t%d", "[", lbracketsym);
                break;
            case procsym:
                printf("%11s\t%d", "procedure", procsym);
                break;
            case rbracketsym:
                printf("%11s\t%d", "]", rbracketsym);
                break;
            case callsym:
                printf("%11s\t%d", "call", callsym);
                break;
            case commasym:
                printf("%11s\t%d", ",", commasym);
                break;
            case beginsym:
                printf("%11s\t%d", "begin", beginsym);
                break;
            case semicolonsym:
                printf("%11s\t%d", ";", semicolonsym);
                break;
            case endsym:
                printf("%11s\t%d", "end", endsym);
                break;
            case assignsym:
                printf("%11s\t%d", ":=", assignsym);
                break;
            case ifsym:
                printf("%11s\t%d", "if", ifsym);
                break;
            case questionsym:
                printf("%11s\t%d", "?", questionsym);
                break;
            case dosym:
                printf("%11s\t%d", "do", dosym);
                break;
            case colonsym:
                printf("%11s\t%d", ":", colonsym);
                break;
            case readsym:
                printf("%11s\t%d", "read", readsym);
                break;
            case lparenthesissym:
                printf("%11s\t%d", "(", lparenthesissym);
                break;
            case writesym:
                printf("%11s\t%d", "write", writesym);
                break;
            case rparenthesissym:
                printf("%11s\t%d", ")", rparenthesissym);
                break;
            case identsym:
                printf("%11s\t%d", list[i].name, identsym);
                break;
            case eqlsym:
                printf("%11s\t%d", "==", eqlsym);
                break;
            case numbersym:
                printf("%11d\t%d", list[i].value, numbersym);
                break;
            case neqsym:
                printf("%11s\t%d", "<>", neqsym);
                break;
            case modsym:
                printf("%11s\t%d", "%", modsym);
                break;
            case lsssym:
                printf("%11s\t%d", "<", lsssym);
                break;
            case divsym:
                printf("%11s\t%d", "/", divsym);
                break;
            case leqsym:
                printf("%11s\t%d", "<=", leqsym);
                break;
            case multsym:
                printf("%11s\t%d", "*", multsym);
                break;
            case gtrsym:
                printf("%11s\t%d", ">", gtrsym);
                break;
            case subsym:
                printf("%11s\t%d", "-", subsym);
                break;
            case geqsym:
                printf("%11s\t%d", ">=", geqsym);
                break;
            case addsym:
                printf("%11s\t%d", "+", addsym);
                break;
            case whilesym:
                printf("%11s\t%d", "while", whilesym);
                break;
            default:
                // printf("%11s\t%s", "sauce", "boss"); // was here
                printf("%11s\t%s", "err", "err");
                break;
        }
        printf("\n");
    }
    printf("\n");
}

void printlexerror(int type) {
    if (type == 1) {
        printf("Lexical Analyzer Error: Invalid Identifier\n");
    } else if (type == 2) {
        printf("Lexical Analyzer Error: Number Length\n");
    } else if (type == 3) {
        printf("Lexical Analyzer Error: Identifier Length\n");
    } else if (type == 4) {
        printf("Lexical Analyzer Error: Invalid Symbol\n");
    } else {
        printf("Implementation Error: Unrecognized Error Type\n");
    }

    free(list);
}
