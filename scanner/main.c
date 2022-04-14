#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#define MAX_BUFF_SIZE 1000
#define TRUE 1
#define FALSE 0
#define BOOLEAN int
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
    SAVE, IGNORE, BACK
}LookAhead;
typedef enum {
    ID, NUM
}TokenType;

FILE* input,* output;//입력 파일과 출력 파일
StateType state = START; //현재 상태
int curLine = 0; //검사중인 라인
TokenType curToken; //현재 토큰 종류
char curTokString[MAX_BUFF_SIZE];   //현재 토큰의 내용
int curTokStringIndex=0;    //현재 토큰 내용의 인덱스

void initInput(int argc, char* argv[]) {
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <input filename> <output filename>\n", argv[0]);
        exit(1);
    }
    input = fopen(argv[1], "r");
    output = fopen(argv[2], "w");
    if (input == NULL) {
        fprintf(OUTPUT,"No input file");
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

void printToken() {
    switch (curToken) {
    case ID:
        fprintf(OUTPUT, "\t%4d: ID, val= %s\n", curLine, curTokString);
        break;
    case NUM:
        fprintf(OUTPUT, "\t%4d: NUM, val= %s\n", curLine, curTokString);
        break;
    }
}

int main(int argc, char* argv[]) {
    initInput(argc,argv);
    char lineHold[MAX_BUFF_SIZE];
    while (!feof(input)) {
        fgets(lineHold, MAX_BUFF_SIZE, input);
        fprintf(OUTPUT, "%4d: %s", ++curLine, lineHold);
        for (int i = 0; lineHold[i]; i++) {
            switch (checkAhead(lineHold[i])) {
            case SAVE:
                curTokString[curTokStringIndex++] = lineHold[i];
                break;
            case IGNORE:
                // do nothing
                break;
            case BACK:
                i--;
                curTokString[curTokStringIndex] = '\0';
                printToken();
                initState();
                break;
            }
        }
    }
    fprintf(OUTPUT,"\t%4d: EOF",++curLine);
}