#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#define MAX_BUFF_SIZE 1000
#define DEBUG
#ifdef DEBUG
#define debug(a) a
#define OUTPUT stdout
#else
#define dubug(a)
#define OUTPUT output
#endif
typedef enum{
    START, INEQ, INCOMMENT, INNUM, INID, DONE, INLT, INGT, INNE, INOVER, INCOMMENT_
}StateType;
typedef enum {
    SAVE, IGNORE, BACK, STAY
}LookAhead;
typedef enum {
    /*0~2*/
    ERROR, ID, NUM,
    /* �����(3~8) */
   ELSE, IF, INT, RETURN, VOID, WHILE,
    /* special symbols(9~27)*/
    PLUS, MINUS, TIMES,DIVIDE,LESS,LESS_EQUAL,GREATER,GREATER_EQUAL, EQUAL, NOT_EQUAL,ASSIGN,SEMICOLON,COMMA, LPAREN, RPAREN, LBRACE, RBRACE, LSQUARE, RSQUARE
}TokenType;
#define RESERVED_TOKEN_COUNT 6
struct {
    char* string;
    TokenType type;
}ReservedToken[RESERVED_TOKEN_COUNT] = { {"else",ELSE},{"if",IF},{"int",INT},{"return",RETURN},{"void",VOID},{"while",WHILE} };

TokenType findKeywords(char* tokenString) {
    for (int i = 0; i < RESERVED_TOKEN_COUNT; i++)
        if (!strcmp(tokenString, ReservedToken[i].string))
            return ReservedToken[i].type;
    return ID;
}

#define FIRST_SPECIAL_SYMBOL PLUS
char specialSymbols[][3] = { "+","-","*","/","<","<=",">",">=","==","!=","=",";",",","(",")","{","}","[","]" };
void printToken() {
    switch (curToken) {
    case ERROR:
        fprintf(OUTPUT, "\t%4d: ERROR: %s\n", curLine, curTokString);
        break;
    case ID:
        fprintf(OUTPUT, "\t%4d: ID, val= %s\n", curLine, curTokString);
        break;
    case NUM:
        fprintf(OUTPUT, "\t%4d: NUM, val= %s\n", curLine, curTokString);
        break;
    default:
        if (curToken < 9)
            fprintf(OUTPUT, "\t%4d: reserved word: % s\n", curLine, curTokString);
        else
            fprintf(OUTPUT, "\t%4d: %s\n", curLine, specialSymbols[curToken- FIRST_SPECIAL_SYMBOL]);
    }
}

FILE* input,* output;//�Է� ���ϰ� ��� ����
StateType state = START; //���� ����
int curLine = 0; //�˻����� ����
TokenType curToken; //���� ��ū ����
char curTokString[MAX_BUFF_SIZE];   //���� ��ū�� ����
int curTokStringIndex=0;    //���� ��ū ������ �ε���

void initInput(int argc, char* argv[]) {
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <input filename> <output filename>\n", argv[0]);
        exit(1);
    }
    input = fopen(argv[1], "r");
    output = fopen(argv[2], "w");
    if (input == NULL) {
        fprintf(OUTPUT,"No input file <%s>",argv[1]);
        exit(1);
    }
}

void initState() {
    state = START;
    curTokStringIndex = 0;
}

LookAhead checkAhead(char c) {
    switch (state) {
    case START:
        if (isdigit(c))
            state = INNUM;
        else if (isalpha(c))
            state = INID;
        else {
            switch (c) {
            case ' ': case '\t': case '\n':
                return IGNORE;
            case '/':
                state = INOVER;
                break;
            case '=':
                state = INEQ;
                break;
            case '<':
                state = INLT;
                break;
            case '>':
                state = INGT;
                break;
            case '!':
                state = INNE;
                break;
            default:
                switch (c) {
                case '+':
                    curToken = PLUS;
                    break;
                case '-':
                    curToken = MINUS;
                    break;
                case '*':
                    curToken = TIMES;
                    break;
                case ';':
                    curToken = SEMICOLON;
                    break;
                case ',':
                    curToken = COMMA;
                    break;
                case '(':
                    curToken = LPAREN;
                    break;
                case ')':
                    curToken = RPAREN;
                    break;
                case '{':
                    curToken = LBRACE;
                    break;
                case '}':
                    curToken = RBRACE;
                    break;
                case '[':
                    curToken = LSQUARE;
                    break;
                case ']':
                    curToken = RSQUARE;
                    break;
                default:
                    curToken = ERROR;
                    break;
                }
                return STAY;
            }
        }
        break; //end of START case

    case INNUM:
        switch (c) {
        case ' ': case '\t': case '\n':
            curToken = NUM;
            return BACK;
        }
        break;

    case INID:
        switch (c) {
        case ' ': case '\t': case '\n':
            curToken = ID;
            return BACK;
        }
        break;
    }
    return SAVE;
}

int main(int argc, char* argv[]) {
    initInput(argc,argv);
    char lineHold[MAX_BUFF_SIZE];
    while (!feof(input)) {
        fgets(lineHold, MAX_BUFF_SIZE, input);
        fprintf(OUTPUT, "%4d: %s", ++curLine, lineHold);
        for (int i = 0; lineHold[i]; i++) {
            switch (checkAhead(lineHold[i])) {
            case SAVE: //���� ���ڸ� ��ū�� �߰�
                curTokString[curTokStringIndex++] = lineHold[i];
                break;
            case IGNORE:
                // do nothing
                break;
            case BACK:  //���� ���� �������� ���� �Ϸ�. �ϼ��� ��ū�� ���� �۾��� ���� ��.
                i--;
            case STAY: //���� ���ڱ��� �ؼ� ���� �Ϸ�.
                curTokString[curTokStringIndex] = '\0';
                curToken = curToken == ID ? findKeywords(curTokString): curToken;
                printToken();

                initState(); //ó�� ���·� �ʱ�ȭ
                break;
            }
        }
    }
    fprintf(OUTPUT,"\t%4d: EOF\n",++curLine);
    return 0;
}