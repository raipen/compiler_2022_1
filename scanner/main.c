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
    START,
    /*숫자나 문자가 들어오는 경우에만 lookAhead를 curToken->string에 SAVE 한다.
    단일 문자 토큰이 들어온 경우 lookAhead를 STAY하고 다시 START로 넘어간다.
    공백 문자를 포함한 그 외의 경우는 모두 IGNORE한다. */
    INNUM, //숫자가 들어오면 시작, 숫자가 들어오면 SAVE, 숫자가 아니면 BACK하고 START로 돌아감
    INID, //문자가 들어오면 시작, 문자가 들어오면 SAVE, 문자가 아니면 BACK하고 START로 돌아감
    INDIVIDE, // '/'가 들어오면 시작, '*'이 오면 IGNORE하고 INCOMMENT로 넘어감, 아니면 그냥 BACK으로 DIVIDE 토큰을 넘기고 START로 돌아간다.
    INCOMMENT, // "/*"가 들어오면 시작, '*'이 들어오면 OUTCOMMENT로 넘어감,항상 모두 IGNORE
    OUTCOMMENT, //INCOMMNET 중 '*'이 들어오면 시작, 바로 이어서 '/'가 들어오면 START, 다른 게 들어오면 INCOMMENT로 돌아감. 항상  IGNORE.
    INLESS, //'<'가 들어오면 시작, '='이 들어오면 STAY하고 아니면 BACK하고 START로 감.
    INGREATER, //'>'가 들어오면 시작, '='이 들어오면 STAY하고 아니면 BACK하고 START로 감.
    INEQ, //'='가 들어오면 시작, '='이 들어오면 STAY하고 아니면 BACK하고 START로 감.
    INNE, //'!'가 들어오면 시작, '='이 들어오면 STAY하고 아니면 BACK하고 START로 감.
}StateType;
typedef enum {
    SAVE, IGNORE, BACK, STAY
    //SAVE는 해당 문자를 토큰문자열에 추가한다.
    //IGNORE은 해당 문자를 무시한다.
    //BACK은 이전 문자까지를 하나의 토큰으로 판단한다.
    //STAY는 현재 문자까지를 하나의 토큰으로 판단한다.
}LookAhead;
typedef enum {
    ENDFILE,ERROR, ID, NUM,
    /* 예약어 */
   ELSE, IF, INT, RETURN, VOID, WHILE,
    /* special symbols*/
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

FILE* input,* output;//입력 파일과 출력 파일
StateType state = START; //현재 상태
int curLine = 0; //검사중인 라인
typedef struct {
    TokenType type; //현재 토큰 종류
    char string[MAX_BUFF_SIZE]; //현재 토큰의 내용
    int index; //현재 토큰 내용의 인덱스
}token;

#define FIRST_SPECIAL_SYMBOL PLUS
char specialSymbols[][3] = { "+","-","*","/","<","<=",">",">=","==","!=","=",";",",","(",")","{","}","[","]" };
void printToken(token* curToken) {

    fprintf(OUTPUT, "\t%d: ",curLine);
    switch (curToken->type) {
    case ENDFILE:
        fprintf(OUTPUT, "EOF\n");
        break;
    case ERROR:
        fprintf(OUTPUT, "ERROR: %s\n", curToken->string);
        break;
    case ID:
        fprintf(OUTPUT, "ID, name= %s\n", curToken->string);
        break;
    case NUM:
        fprintf(OUTPUT, "NUM, val= %s\n", curToken->string);
        break;
    default:
        if (curToken->type < FIRST_SPECIAL_SYMBOL)
            fprintf(OUTPUT, "reserved word: % s\n", curToken->string);
        else
            fprintf(OUTPUT, "%s\n", specialSymbols[curToken->type- FIRST_SPECIAL_SYMBOL]);
    }
}

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
}

LookAhead checkAhead(char c,token* curToken) {
    switch (state) {
    case START:
        if (isdigit(c)) //숫자가 들어오면 INNUM으로 변경 후 SAVE 리턴
            state = INNUM;
        else if (isalpha(c)) //문자가 들어오면 INID로 변경 후 SAVE 리턴
            state = INID;
        else {
            switch (c) {//여기서 case에 해당되면 IGNORE
            case ' ': case '\t': case '\n':
                break;
            case '/':
                state = INDIVIDE;
                break;
            case '=':
                state = INEQ;
                break;
            case '<':
                state = INLESS;
                break;
            case '>':
                state = INGREATER;
                break;
            case '!':
                state = INNE;
                break;
            default:
                switch (c) { //case에 해당되면 모두 STAY
                case '+':
                    curToken->type = PLUS;
                    break;
                case '-':
                    curToken->type = MINUS;
                    break;
                case '*':
                    curToken->type = TIMES;
                    break;
                case ';':
                    curToken->type = SEMICOLON;
                    break;
                case ',':
                    curToken->type = COMMA;
                    break;
                case '(':
                    curToken->type = LPAREN;
                    break;
                case ')':
                    curToken->type = RPAREN;
                    break;
                case '{':
                    curToken->type = LBRACE;
                    break;
                case '}':
                    curToken->type = RBRACE;
                    break;
                case '[':
                    curToken->type = LSQUARE;
                    break;
                case ']':
                    curToken->type = RSQUARE;
                    break;
                default:
                    curToken->type = ERROR;
                    curToken->string[curToken->index++] = c;
                    break;
                }
                return STAY;
            }
            return IGNORE;
        }
        return SAVE;
        break; //end of START case


    case INNUM: //숫자가 들어오면 시작, 숫자가 들어오면 SAVE, 숫자가 아니면 BACK하고 START로 돌아감
        if (!isdigit(c)) {
            curToken->type = NUM;
            return BACK;
        }
        return SAVE;
    case INID: //문자가 들어오면 시작, 문자가 들어오면 SAVE, 문자가 아니면 BACK하고 START로 돌아감
        if (!isalpha(c)) {
            curToken->type = ID;
            return BACK;
        }
        return SAVE;
        break;
    case INDIVIDE: // '/'가 들어오면 시작, '*'이 오면 IGNORE하고 INCOMMENT로 넘어감, 아니면 그냥 BACK으로 DIVIDE 토큰을 넘기고 START로 돌아간다.
        switch (c) {
        case '*':
            state = INCOMMENT;
            return IGNORE;
        default:
            curToken->type = DIVIDE;
            return BACK;
        }
    case INCOMMENT: // "/*"가 들어오면 시작, '*'이 들어오면 OUTCOMMENT로 넘어감,항상 모두 IGNORE
        switch (c) {
        case '*':
            state = OUTCOMMENT;
        default:
            return IGNORE;
        }
    case OUTCOMMENT: //INCOMMNET 중 '*'이 들어오면 시작, 바로 이어서 '/'가 들어오면 START, '*'이 들어오면 그대로 OUTCOMMENT, 다른 게 들어오면 INCOMMENT로 돌아감. 항상  IGNORE.
        switch (c) {
        case '/':
            state = START;
            return IGNORE;
        case '*':
            return IGNORE;
        default:
            state = INCOMMENT;
            return IGNORE;
        }
    case INLESS: //'<'가 들어오면 시작, '='이 들어오면 STAY하고 아니면 BACK하고 START로 감.
        switch (c) {
        case '=':
            curToken->type = LESS_EQUAL;
            return STAY;
        default:
            curToken->type = LESS;
            return BACK;
        }
        break;
    case INGREATER: //'>'가 들어오면 시작, '='이 들어오면 STAY하고 아니면 BACK하고 START로 감.
        switch (c) {
        case '=':
            curToken->type = GREATER_EQUAL;
            return STAY;
        default:
            curToken->type = GREATER;
            return BACK;
        }
        break;
    case INEQ: //'='가 들어오면 시작, '='이 들어오면 STAY하고 아니면 BACK하고 START로 감.
        switch (c) {
        case '=':
            curToken->type = EQUAL;
            return STAY;
        default:
            curToken->type = ASSIGN;
            return BACK;
        }
        break;
    case INNE: //'!'가 들어오면 시작, '='이 들어오면 STAY하고 아니면 BACK하고 START로 감.
        switch (c) {
        case '=':
            curToken->type = NOT_EQUAL;
            return STAY;
        default:
            curToken->type = ERROR;
            curToken->string[curToken->index++] = '!';
            return BACK;
        }
        break;
    }
}

char lineHold[MAX_BUFF_SIZE] = { 0 }; //입력파일로 부터 받아온 문자열 저장
int buff_index = 0;

char getAhead() {
    if (!lineHold[buff_index]) { //버퍼에 있는 문자를 모두 확인 했다면
        if (fgets(lineHold, MAX_BUFF_SIZE, input) == NULL) return EOF;  //다음줄을 받아옴. 이때 더이상 받아올 줄이 없다면 EOF 반환
        fprintf(OUTPUT, "%2d: %s", ++curLine, lineHold);    //해당 줄을 화면에 출력
        if (feof(input)) fprintf(OUTPUT, "\n"); //파일이 엔터 없이 끝난 경우. 엔터를 따로 출력해줌
        buff_index = 0; //그 라인의 첫번째 값부터 확인하도록 초기화
    }
    return lineHold[buff_index++]; //버퍼에 있는 다음 문자를 리턴
}

void backAhead() {
    buff_index--;   //해당 문자를 읽지 않은 상태로 되돌림
}

token getToken() {
    token curToken = { ENDFILE,"",0 };
    while (1) {
        char c = getAhead();
        if (c == EOF) {         //다음 문자가 파일의 끝이라면
            curLine++;          //다음줄로 넘어가고
            return curToken; //ENDFILE 토큰을 리턴
        }
        switch (checkAhead(c, &curToken)) {     //lookahead를 이용하여 어떻게 처리할 지 체크함.
            /*SAVE는 해당 문자를 토큰문자열에 추가한다.
            IGNORE은 해당 문자를 무시한다.
            BACK은 이전 문자까지를 하나의 토큰으로 판단한다.
            STAY는 현재 문자까지를 하나의 토큰으로 판단한다.*/
        case SAVE: //현재 문자를 토큰에 추가
            curToken.string[curToken.index++] = c;
            break;
        case IGNORE:
            // do nothing
            break;
        case BACK:  //현재 문자 이전에서 토큰 리턴.
            backAhead();
        case STAY: //현재 문자까지 포함하여 토큰 리턴.
            curToken.string[curToken.index] = '\0';
            curToken.type = curToken.type == ID ? findKeywords(curToken.string) : curToken.type;
            initState(); //처음 상태로 초기화
            return curToken;
        }
    }
}


int main(int argc, char* argv[]) {
    initInput(argc,argv);
    token curToken;
    do {
        curToken = getToken();
        printToken(&curToken);
    } while (curToken.type != ENDFILE);
    return 0;
}