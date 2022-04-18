
/*
 Group Members:

 Jan Darge
 Raciel Antela Pardo
 Celina Alzenor
 */

#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CODE_LENGTH 200
#define MAX_SYMBOL_COUNT 50
#define MAX_REG_HEIGHT 10
#define MAX_SYMBOLNAME_LENGTH 12

instruction *code;
symbol *table;
int codeIdx;
int tableIdx;
int listIdx;
int level;
int rCounter;

void emit(int opname, int reg, int level, int mvalue);
void addToSymbolTable(int k, char n[], int s, int l, int a, int m);
void mark();
int mult_dec(char name[]);
int findsymbol(char name[], int kind);
void printparseerror(int err_code);
void printsymboltable();
void printassemblycode();

void block(lexeme *list);
int var_dec(lexeme *list);
void proc_dec(lexeme *list);
void statement(lexeme *list);
void condition(lexeme *list);
void expression(lexeme *list);
void term(lexeme *list);
void factor(lexeme *list);

instruction *parse(lexeme *list, int printTable, int printCode) {
    code = (instruction *)malloc(sizeof(instruction) * MAX_CODE_LENGTH);
    table = (symbol *)malloc(sizeof(symbol) * MAX_SYMBOL_COUNT);

    codeIdx = 0;
    tableIdx = 0;
    listIdx = 0;
    level = 0;
    rCounter = -1;

    emit(7, 0, 0, 0);
    addToSymbolTable(3, "main", 0, 0, 0, 0);

    level = -1;

    block(list);

    if(list[listIdx].type != periodsym) {
        printparseerror(1);// TODO this is not working?
    }

    emit(11, 0, 0, 0);

    code[0].m = table[0].addr;

    for (int i = 0; i < codeIdx; i++) {
        if (code[i].opcode == 5) {
            code[i].m = table[code[i].m].addr;
        }
    }

    if (printTable)
        printsymboltable();
    if (printCode)
        printassemblycode();

    code[codeIdx].opcode = -1;
    return code;
}

void block(lexeme *list) {
    level++;

    int procedureIndex = tableIdx - 1;
    int x = var_dec(list);
    proc_dec(list);

    table[procedureIndex].addr = codeIdx;
    emit(6, 0, 0, x); // emit INC
    statement(list);

    mark();

    level--;
}

int var_dec(lexeme *list) {

    int memorysize = 3;
    char symbolname[MAX_SYMBOLNAME_LENGTH];
    int arraysize;

    if (list[listIdx].type == varsym) {
        do {
            listIdx++;
            if (list[listIdx].type != identsym) {
                printparseerror(2);
            }
            if (mult_dec(list[listIdx].name) != -1) {
                printparseerror(3);
            }
            strcpy(symbolname, list[listIdx].name);
            listIdx++;

            if (list[listIdx].type == lbracketsym) {
                listIdx++;
                if (list[listIdx].type != numbersym ||
                    list[listIdx].value == 0) {
                    printparseerror(4);
                }

                arraysize = list[listIdx].value;
                listIdx++;

                if (list[listIdx].type == multsym ||
                    list[listIdx].type == divsym ||
                    list[listIdx].type == modsym ||
                    list[listIdx].type == addsym ||
                    list[listIdx].type == subsym) {
                    printparseerror(4);
                } else if (list[listIdx].type != rbracketsym) {
                    printparseerror(5);
                }

                listIdx++;
                addToSymbolTable(2, symbolname, arraysize, level, memorysize, 0);
                memorysize += arraysize;
            } else {
                addToSymbolTable(1, symbolname, 0, level, memorysize, 0);
                memorysize++;
            }
        } while (list[listIdx].type == commasym);

        if (list[listIdx].type == identsym) {
            printparseerror(6);
        } else if (list[listIdx].type != semicolonsym) {
            printparseerror(7);
        }

        listIdx++;
        return memorysize;
    } else {
        return memorysize;
    }
}

void proc_dec(lexeme *list) {

    char symbolname[MAX_SYMBOLNAME_LENGTH];

    while (list[listIdx].type == procsym) {
        listIdx++;

        if (list[listIdx].type != identsym) {
            printparseerror(2);
        } else if (mult_dec(list[listIdx].name) != -1) {
            printparseerror(3);
        }

        strcpy(symbolname, list[listIdx].name);
        listIdx++;

        if (list[listIdx].type != semicolonsym) {
            printparseerror(8);
        }

        listIdx++;
        addToSymbolTable(3, symbolname, 0, level, 0, 0);
        block(list);

        if (list[listIdx].type != semicolonsym) {
            printparseerror(7);
        }

        listIdx++;
        emit(2, 0, 0, 0);
    }
}

void statement(lexeme *list) {

    char symbolname[MAX_SYMBOLNAME_LENGTH];
    int symidx;  // symbol index
    int arridx;  // array reg index
    int varidx;  // var location reg
    int jpcidx;  // jpc index
    int jmpidx;  // jmp index
    int loopidx; // loop location

    if (list[listIdx].type == identsym) {
        strcpy(symbolname, list[listIdx].name);
        listIdx++;
        if (list[listIdx].type == lbracketsym) {

            listIdx++;
            symidx = findsymbol(symbolname, 2);

            if (symidx == -1) {
                if (findsymbol(symbolname, 1) != -1) {
                    printparseerror(11);
                }

                else if (findsymbol(symbolname, 3) != -1) {
                    printparseerror(9);
                }

                else {
                    printparseerror(10);
                }
            }

            expression(list);
            arridx = rCounter;

            if (list[listIdx].type != rbracketsym) {
                printparseerror(5);
            }

            listIdx++;

            if (list[listIdx].type != assignsym) {
                printparseerror(13);
            }

            listIdx++;
            expression(list);
            rCounter++;

            if (rCounter >= 10) {
                printparseerror(14);
            }

            emit(1, rCounter, 0, table[symidx].addr); // emit LIT
            emit(13, arridx, arridx, rCounter);       // emit ADD
            rCounter--;
            emit(4, rCounter, level - table[symidx].level, arridx); // emit STO
            rCounter -= 2;
        } else {
            symidx = findsymbol(symbolname, 1);

            if (symidx == -1) {
                if (findsymbol(symbolname, 2) != -1) {
                    printparseerror(12);
                }

                else if (findsymbol(symbolname, 3) != -1) {
                    printparseerror(9);
                }

                else {
                    printparseerror(10); // TODO error1.txt yeeted here
                }
            }

            rCounter++;

            if (rCounter >= 10) {
                printparseerror(14);
            }

            emit(1, rCounter, 0, table[symidx].addr);
            varidx = rCounter;

            if (list[listIdx].type != assignsym) {
                printparseerror(13);
            }

            listIdx++;
            expression(list);
            emit(4, rCounter, level - table[symidx].level, varidx);
            rCounter -= 2;
        }
    } else if (list[listIdx].type == callsym) {
        listIdx++;
        if (list[listIdx].type != identsym) {
            printparseerror(15);
        }
        symidx = findsymbol(list[listIdx].name, 3);
        if (symidx == -1) {
            if (findsymbol(list[listIdx].name, 1) != -1 ||
                findsymbol(list[listIdx].name, 2) != -1) {
                printparseerror(15);
            } else {
                printparseerror(10);
            }
        }
        emit(5, 0, level - table[symidx].level, symidx); // emit CAL
        listIdx++;
    } else if (list[listIdx].type == beginsym) {
        do {
            listIdx++;
            statement(list);
        } while (list[listIdx].type == semicolonsym);
        if (list[listIdx].type != endsym) {
            if (list[listIdx].type == identsym ||
                list[listIdx].type == callsym ||
                list[listIdx].type == beginsym ||
                list[listIdx].type == ifsym ||
                list[listIdx].type == dosym ||
                list[listIdx].type == readsym ||
                list[listIdx].type == writesym) {
                printparseerror(16);
            } else {
                printparseerror(17);
            }
        }
        listIdx++;
    } else if (list[listIdx].type == ifsym) {
        listIdx++;
        condition(list);
        jpcidx = codeIdx;
        emit(8, rCounter, 0, 0); // emit JPC
        rCounter--;

        if (list[listIdx].type != questionsym) {
            printparseerror(18);
        }

        listIdx++;
        statement(list);

        if (list[listIdx].type == colonsym) {
            listIdx++;
            jmpidx = codeIdx;
            emit(7, 0, 0, 0); // emit JPC
            code[jpcidx].m = codeIdx;
            statement(list);
            code[jmpidx].m = codeIdx;
        } else {
            code[jpcidx].m = codeIdx;
        }
    } else if (list[listIdx].type == dosym) {// TODO error 19 doesnt work properly?
        listIdx++;
        loopidx = codeIdx;
        statement(list);
        if (list[listIdx].type != whilesym) {
            printparseerror(19);
        }
        listIdx++;
        condition(list);
        rCounter++;
        if (rCounter >= 10) {
            printparseerror(14);
        }
        emit(1, rCounter, 0, 0);                        // emit LIT
        emit(18, rCounter - 1, rCounter - 1, rCounter); // emit EQL
        rCounter--;
        emit(8, rCounter, 0, loopidx); // emit JPC
        rCounter--;
    } else if (list[listIdx].type == readsym) {
        listIdx++;

        if (list[listIdx].type != identsym) {
            printparseerror(20);
        }

        strcpy(symbolname, list[listIdx].name);
        listIdx++;

        if (list[listIdx].type == lbracketsym) {
            listIdx++;
            symidx = findsymbol(symbolname, 2);

            if (symidx == -1) {
                if (findsymbol(symbolname, 1) != -1) {
                    printparseerror(11);
                } else if (findsymbol(symbolname, 3) != -1) {
                    printparseerror(9);
                } else {
                    printparseerror(10);
                }
            }

            expression(list);
            arridx = rCounter;

            if (list[listIdx].type != rbracketsym) {
                printparseerror(5);
            }

            listIdx++;
            rCounter++;

            if (rCounter >= 10) {
                printparseerror(14);
            }

            emit(10, rCounter, 0, 0); // emit RED
            rCounter++;

            if (rCounter >= 10) {
                printparseerror(14);
            }

            emit(1, rCounter, 0, table[symidx].addr); // emit LIT
            emit(13, arridx, arridx, rCounter);       // emit ADD
            rCounter--;
            emit(4, rCounter, level - table[symidx].level, arridx); // emit STO
            rCounter -= 2;

        } else {

            symidx = findsymbol(symbolname, 1);

            if (symidx == -1) {
                if (findsymbol(symbolname, 2) != -1) {
                    printparseerror(12);
                } else if (findsymbol(symbolname, 3) != -1) {
                    printparseerror(9);
                } else {
                    printparseerror(10);
                }
            }

            rCounter++;

            if (rCounter >= 10) {
                printparseerror(14);
            }

            emit(1, rCounter, 0, table[symidx].addr); // emit LIT
            varidx = rCounter;
            rCounter++;

            if (rCounter >= 10) {
                printparseerror(14);
            }

            emit(10, rCounter, 0, 0);                               // emit RED
            emit(4, rCounter, level - table[symidx].level, varidx); // emit STO
            rCounter -= 2;
        }
    } else if (list[listIdx].type == writesym) {
        listIdx++;
        expression(list);
        emit(9, rCounter, 0, 0); // emit WRT
        rCounter--;
    }
}

void condition(lexeme *list) {
    expression(list);
    if (list[listIdx].type == eqlsym) {
        listIdx++;
        expression(list);
        emit(18, rCounter - 1, rCounter - 1, rCounter); // emit EQL
        rCounter--;
    } else if (list[listIdx].type == neqsym) {
        listIdx++;
        expression(list);
        emit(19, rCounter - 1, rCounter - 1, rCounter); // emit NEQ
        rCounter--;
    } else if (list[listIdx].type == lsssym) {
        listIdx++;
        expression(list);
        emit(20, rCounter - 1, rCounter - 1, rCounter); // emit LSS
        rCounter--;
    } else if (list[listIdx].type == leqsym) {
        listIdx++;
        expression(list);
        emit(21, rCounter - 1, rCounter - 1, rCounter); // emit LEQ
        rCounter--;
    } else if (list[listIdx].type == gtrsym) {
        listIdx++;
        expression(list);
        emit(22, rCounter - 1, rCounter - 1, rCounter); // emit GRT
        rCounter--;
    } else if (list[listIdx].type == geqsym) {
        listIdx++;
        expression(list);
        emit(23, rCounter - 1, rCounter - 1, rCounter); // emit GEQ
        rCounter--;
    } else {
        printparseerror(21);
    }
}

// Handles addition and subtraction
void expression(lexeme *list) {
    if (list[listIdx].type == subsym) {
        listIdx++;
        term(list);
        emit(12, rCounter, 0, rCounter); // emit NEG
        while (list[listIdx].type == addsym || list[listIdx].type == subsym) {
            if (list[listIdx].type == addsym) {
                listIdx++;
                term(list);
                emit(13, rCounter - 1, rCounter - 1, rCounter); // emit ADD
                rCounter--;
            } else {
                listIdx++;
                term(list);
                emit(14, rCounter - 1, rCounter - 1, rCounter); // emit SUB
                rCounter--;
            }
        }
    } else {
        term(list);
        while (list[listIdx].type == addsym || list[listIdx].type == subsym) {
            if (list[listIdx].type == addsym) {
                listIdx++;
                term(list);
                emit(13, rCounter - 1, rCounter - 1, rCounter); // emit ADD
                rCounter--;
            } else {
                listIdx++;
                term(list);
                emit(14, rCounter - 1, rCounter - 1, rCounter); // emit SUB
                rCounter--;
            }
        }
    }

    if (list[listIdx].type == lparenthesissym ||
        list[listIdx].type == identsym || list[listIdx].type == numbersym) {
        printparseerror(22);
    }
}

// Handles multiplication, division and modulus
void term(lexeme *list) {
    factor(list);
    while (list[listIdx].type == multsym || list[listIdx].type == divsym ||
           list[listIdx].type == modsym) {
        if (list[listIdx].type == multsym) {
            listIdx++;
            factor(list);
            emit(15, rCounter - 1, rCounter - 1, rCounter); // emit MUL
            rCounter--;
        } else if (list[listIdx].type == divsym) {
            listIdx++;
            factor(list);
            emit(16, rCounter - 1, rCounter - 1, rCounter); // emit DIV
            rCounter--;
        } else {
            listIdx++;
            factor(list);
            emit(17, rCounter - 1, rCounter - 1, rCounter); // emit MOD
            rCounter--;
        }
    }
}

// Handles addition and subtraction
void factor(lexeme *list) {
    char symbolname[MAX_SYMBOLNAME_LENGTH];
    int symidx;
    int arridx;
    int varidx;

    if (list[listIdx].type == identsym) {
        strcpy(symbolname, list[listIdx].name);
        listIdx++;
        if (list[listIdx].type == lbracketsym) {
            listIdx++;
            symidx = findsymbol(symbolname, 2);
            if (symidx == -1) {
                if (findsymbol(symbolname, 1) != -1) {
                    printparseerror(11);
                } else if (findsymbol(symbolname, 3) != -1) {
                    printparseerror(9);
                } else {
                    printparseerror(10);
                }
            }
            expression(list);
            arridx = rCounter;
            if (list[listIdx].type != rbracketsym) {
                printparseerror(5);
            }
            listIdx++;
            rCounter++;
            if (rCounter >= 10) {
                printparseerror(14);
            }
            emit(1, rCounter, 0, table[symidx].addr); // emit LIT
            emit(13, arridx, arridx, rCounter);       // emit ADD
            rCounter--;
            emit(3, rCounter, level - table[symidx].level, arridx); // emit LOD
        } else {
            symidx = findsymbol(symbolname, 1);
            if (symidx == -1) {
                if (findsymbol(symbolname, 2) != 1) {
                    printparseerror(12);
                } else if (findsymbol(symbolname, 3) != 1) {
                    printparseerror(9);
                } else {
                    printparseerror(10);
                }
            }
            rCounter++;
            if (rCounter >= 10) {
                printparseerror(14);
            }
            emit(1, rCounter, 0, table[symidx].addr); // emit LIT
            varidx = rCounter;
            emit(3, rCounter, level - table[symidx].level, varidx); // emit LOD
        }
    }

    else if (list[listIdx].type == numbersym) {
        rCounter++;
        if (rCounter >= 10) {
            printparseerror(14);
        }
        emit(1, rCounter, 0, list[listIdx].value); // emit LIT
        listIdx++;
    }

    else if (list[listIdx].type == lparenthesissym) { // TODO error 23 fails?
        listIdx++;
        expression(list);
        if (list[listIdx].type != rparenthesissym) {
            printparseerror(23);
        }
        listIdx++;
    }

    else {
        printparseerror(24);
    }
}

void emit(int opname, int reg, int level, int mvalue) {
    code[codeIdx].opcode = opname;
    code[codeIdx].r = reg;
    code[codeIdx].l = level;
    code[codeIdx].m = mvalue;
    codeIdx++;
}

void addToSymbolTable(int k, char n[], int s, int l, int a, int m) {
    table[tableIdx].kind = k;
    strcpy(table[tableIdx].name, n);
    table[tableIdx].size = s;
    table[tableIdx].level = l;
    table[tableIdx].addr = a;
    table[tableIdx].mark = m;
    tableIdx++;
}

// Mark symbols in symbol table
void mark() {
    int i;
    for (i = tableIdx - 1; i >= 0; i--) {
        if (table[i].mark == 1)
            continue;
        if (table[i].level < level)
            return;
        table[i].mark = 1;
    }
}

// Find exising symbol in symbol table
int mult_dec(char name[]) {
    int i;
    for (i = 0; i < tableIdx; i++)
        if (table[i].mark == 0 && table[i].level == level &&
            strcmp(name, table[i].name) == 0)
            return i;
    return -1;
}

int findsymbol(char name[], int kind) {
    int i;
    int max_idx = -1;
    int max_lvl = -1;
    for (i = 0; i < tableIdx; i++) {
        if (table[i].mark == 0 && table[i].kind == kind &&
            strcmp(name, table[i].name) == 0) {
            if (max_idx == -1 || table[i].level > max_lvl) {
                max_idx = i;
                max_lvl = table[i].level;
            }
        }
    }
    return max_idx;
}

void printparseerror(int err_code) {
    switch (err_code) {
    case 1:
        printf("Parser Error: Program must be closed by a period\n");
        break;
    case 2:
        printf("Parser Error: Symbol names must be identifiers\n");
        break;
    case 3:
        printf("Parser Error: Confliciting symbol declarations\n");
        break;
    case 4:
        printf("Parser Error: Array sizes must be given as a single, "
               "nonzero "
               "number\n");
        break;
    case 5:
        printf("Parser Error: [ must be followed by ]\n");
        break;
    case 6:
        printf("Parser Error: Multiple symbols in var declaration must be "
               "separated by commas\n");
        break;
    case 7:
        printf("Parser Error: Symbol declarations should close with a "
               "semicolon\n");
        break;
    case 8:
        printf("Parser Error: Procedure declarations should contain a "
               "semicolon before the body of the procedure begins\n");
        break;
    case 9:
        printf("Parser Error: Procedures may not be assigned to, read, "
               "or used "
               "in arithmetic\n");
        break;
    case 10:
        printf("Parser Error: Undeclared identifier\n");
        break;
    case 11:
        printf("Parser Error: Variables cannot be indexed\n");
        break;
    case 12:
        printf("Parserr Error: Arrays must be indexed\n");
        break;
    case 13:
        printf("Parser Error: Assignment operator missing\n");
        break;
    case 14:
        printf("Parser Error: Register Overflow Error\n");
        break;
    case 15:
        printf("Parser Error: call must be followed by a procedure "
               "identifier\n");
        break;
    case 16:
        printf("Parser Error: Statements within begin-end must be "
               "separated by "
               "a semicolon\n");
        break;
    case 17:
        printf("Parser Error: begin must be followed by end\n");
        break;
    case 18:
        printf("Parser Error: if must be followed by ?\n");
        break;
    case 19:
        printf("Parser Error: do must be followed by while\n");
        break;
    case 20:
        printf("Parser Error: read must be followed by a var or array "
               "identifier\n");
        break;
    case 21:
        printf("Parser Error: Relational operator missing from "
               "condition\n");
        break;
    case 22:
        printf("Parser Error: Bad arithmetic\n");
        break;
    case 23:
        printf("Parser Error: ( must be followed by )\n");
        break;
    case 24:
        printf("Parser Error: Arithmetic expressions may only contain "
               "arithmetic operators, numbers, parentheses, and "
               "variables\n");
        break;
    default:
        printf("Implementation Error: unrecognized error code\n");
        break;
    }

    free(code);
    free(table);

    exit(0);
}

void printsymboltable() {
    int i;
    printf("Symbol Table:\n");
    printf("Kind | Name        | Size | Level | Address | Mark\n");
    printf("---------------------------------------------------\n");
    for (i = 0; i < tableIdx; i++)
        printf("%4d | %11s | %5d | %4d | %5d | %5d\n", table[i].kind,
               table[i].name, table[i].size, table[i].level, table[i].addr,
               table[i].mark);

    free(table);
    table = NULL;
}

void printassemblycode() {
    int i;
    printf("Line\tOP Code\tOP Name\tR\tL\tM\n");
    for (i = 0; i < codeIdx; i++) {
        printf("%d\t", i);
        printf("%d\t", code[i].opcode);
        switch (code[i].opcode) {
        case 1:
            printf("LIT\t");
            break;
        case 2:
            printf("RET\t");
            break;
        case 3:
            printf("LOD\t");
            break;
        case 4:
            printf("STO\t");
            break;
        case 5:
            printf("CAL\t");
            break;
        case 6:
            printf("INC\t");
            break;
        case 7:
            printf("JMP\t");
            break;
        case 8:
            printf("JPC\t");
            break;
        case 9:
            printf("WRT\t");
            break;
        case 10:
            printf("RED\t");
            break;
        case 11:
            printf("HLT\t");
            break;
        case 12:
            printf("NEG\t");
            break;
        case 13:
            printf("ADD\t");
            break;
        case 14:
            printf("SUB\t");
            break;
        case 15:
            printf("MUL\t");
            break;
        case 16:
            printf("DIV\t");
            break;
        case 17:
            printf("MOD\t");
            break;
        case 18:
            printf("EQL\t");
            break;
        case 19:
            printf("NEQ\t");
            break;
        case 20:
            printf("LSS\t");
            break;
        case 21:
            printf("LEQ\t");
            break;
        case 22:
            printf("GTR\t");
            break;
        case 23:
            printf("GEQ\t");
            break;
        default:
            printf("err\t");
            break;
        }
        printf("%d\t%d\t%d\n", code[i].r, code[i].l, code[i].m);
    }

    if (table != NULL) {
        free(table);
    }
}
