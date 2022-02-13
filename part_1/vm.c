/*
 * TEAM MEMBER NAMES:
 *  - Raciel Antela Pardo
 *  - Jan Darge
 *  - Celina Alzenor
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

#define REG_FILE_SIZE 10
#define MAX_STACK_LENGTH 100

#define ERROR_SO \
    do { \
        printf("Virtual Machine Error: Out of Bounds Access Error\n"); \
        goto HALT; \
    } while (0)

#define ERROR_OB \
    do { \
        printf("Virtual Machine Error: Stack Overflow Error\n"); \
        goto HALT; \
    } while (0)

void print_execution(int line, char *opname, instruction IR, int PC, int BP, int SP, int *stack, int *RF) {
    int i;
    // print out instruction and registers
    printf("%2d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t\t", line, opname, IR.r, IR.l, IR.m, PC, SP, BP);

    // print register file
    for (i = 0; i < REG_FILE_SIZE; i++)
        printf("%d ", RF[i]);
    printf("\n");

    // print stack
    printf("stack:\t");
    for (i = MAX_STACK_LENGTH - 1; i >= SP; i--)
        printf("%d ", stack[i]);
    printf("\n");
}

int base(int l, int bp, const int *stack) {
    int ctr = l;
    int rtn = bp;
    while (ctr > 0) {
        rtn = stack[rtn];
        ctr--;
    }
    return rtn;
}

char *opname(int OP) {

    switch (OP) {
        case 1:
            return "LIT";
        case 2:
            return "RET";
        case 3:
            return "LOD";
        case 4:
            return "STO";
        case 5:
            return "CAL";
        case 6:
            return "INC";
        case 7:
            return "JMP";
        case 8:
            return "JPC";
        case 9:
            return "WRT";
        case 10:
            return "RED";
        case 11:
            return "HLT";
        case 12:
            return "NEG";
        case 13:
            return "ADD";
        case 14:
            return "SUB";
        case 15:
            return "MUL";
        case 16:
            return "DIV";
        case 17:
            return "MOD";
        case 18:
            return "EQL";
        case 19:
            return "NEQ";
        case 20:
            return "LSS";
        case 21:
            return "LEQ";
        case 22:
            return "GTR";
        case 23:
            return "GEQ";
        default:
            return NULL;
    }
}

void execute_program(instruction *IR, int printFlag) {

    int BP = MAX_STACK_LENGTH - 1;
    int SP = BP + 1;
    int PC = 0;

    int *stack = (int *) calloc(MAX_STACK_LENGTH, sizeof(int));
    int *RF = (int *) calloc(REG_FILE_SIZE, sizeof(int));

    // for RED user input
    char *ptr, *str = (char *) malloc(sizeof(char) * 5);

    int start_pos;
    int pos = 0; // our overall position

    int r, l, m;
    int halt = 0; // halt flag

    // keep this
    if (printFlag) {
        printf("\t\t\t\t\tPC\tSP\tBP\n");
        printf("Initial values:\t\t\t\t%d\t%d\t%d\n", PC, SP, BP);
    }

    while (!halt) {

        start_pos = pos;

        r = IR[pos].r; // current register
        l = IR[pos].l; // current lexicographical level
        m = IR[pos].m; //

        switch (IR[pos].opcode) {

            case 1: { // LIT
                RF[r] = m;
                break;
            }

            case 2: { // RET

                if (BP == 1)
                    goto HALT;

                SP = BP + 1;
                BP = stack[SP - 2]; // dynamic link
                PC = stack[SP - 3] - 1; // return address
                pos = PC;

                break;
            }

            case 3: { // LOD

                int val = base(l, BP, stack);

                if ((val - RF[m]) >= MAX_STACK_LENGTH || (val - RF[m]) < 0) {
                    ERROR_SO;
                }

                RF[r] = stack[val - RF[m]];

                break;
            }

            case 4: { // STO

                int val = base(l, BP, stack);

                if ((val - RF[m]) >= MAX_STACK_LENGTH || (val - RF[m]) < 0) {
                    ERROR_SO;
                }

                stack[val - RF[m]] = RF[r];

                break;
            }

            case 5: { // CAL

                stack[SP - 1] = base(l, BP, stack); //stack link
                stack[SP - 2] = BP;   //dynamic link
                stack[SP - 3] = PC + 1;   // return address

                BP = SP - 1;
                PC = m - 1;
                pos = m - 1;

                break;
            }

            case 6: { // INC
                SP -= m;
                if (SP < 0)
                    ERROR_OB;
                break;
            }

            case 7: { // JMP
                pos = m - 1;
                PC = m - 1;
                break;
            }

            case 8: { // JPC

                if (!RF[r]) {
                    pos = m - 1;
                    PC = m - 1;
                }

                break;
            }

            case 9: { // WRT
                printf("Write Value: %d\n", RF[r]);
                break;
            }

            case 10: { // RED

                printf("Please Enter a Value:\n");
                scanf("%s", str);

                // This is for actual user inputs
                // int temp;
                // while ((temp = getchar() != '\n') && temp != EOF);

                RF[r] = (int) strtol(str, &ptr, 10);

                break;
            }

            case 11: { // HLT
                HALT:
                halt = 1;
                break;
            }

            case 12: { // NEG
                RF[r] = -RF[r];
                break;
            }

            case 13: { // ADD
                RF[r] = RF[l] + RF[m];
                break;
            }

            case 14: { // SUB
                RF[r] = RF[l] - RF[m];
                break;
            }

            case 15: { // MUL
                RF[r] = RF[l] * RF[m];
                break;
            }

            case 16: { // DIV
                RF[r] = RF[l] / RF[m];
                break;
            }

            case 17: { // MOD
                RF[r] = RF[l] % RF[m];
                break;
            }

            case 18: { // EQL
                RF[r] = (RF[l] == RF[m]) ? 1 : 0;
                break;
            }

            case 19: { // NEQ
                RF[r] = (RF[l] != RF[m]) ? 1 : 0;
                break;
            }

            case 20: { // LSS
                RF[r] = (RF[l] < RF[m]) ? 1 : 0;
                break;
            }

            case 21: { // LEQ
                RF[r] = (RF[l] <= RF[m]) ? 1 : 0;
                break;
            }

            case 22: { // GTR
                RF[r] = (RF[l] > RF[m]) ? 1 : 0;
                break;
            }

            case 23: { // GEQ
                RF[r] = (RF[l] >= RF[m]) ? 1 : 0;
                break;
            }
        } // END SWITCH

        ++PC;
        ++pos;

        strcpy(str, opname(IR[start_pos].opcode));
        print_execution(start_pos, str, IR[start_pos], PC, BP, SP, stack, RF);
    } // END WHILE

    free(str);
}