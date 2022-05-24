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
    /*���ڳ� ���ڰ� ������ ��쿡�� lookAhead�� curToken->string�� SAVE �Ѵ�.
    ���� ���� ��ū�� ���� ��� lookAhead�� STAY�ϰ� �ٽ� START�� �Ѿ��.
    ���� ���ڸ� ������ �� ���� ���� ��� IGNORE�Ѵ�. */
    INNUM, //���ڰ� ������ ����, ���ڰ� ������ SAVE, ���ڰ� �ƴϸ� BACK�ϰ� START�� ���ư�
    INID, //���ڰ� ������ ����, ���ڰ� ������ SAVE, ���ڰ� �ƴϸ� BACK�ϰ� START�� ���ư�
    INDIVIDE, // '/'�� ������ ����, '*'�� ���� IGNORE�ϰ� INCOMMENT�� �Ѿ, �ƴϸ� �׳� BACK���� DIVIDE ��ū�� �ѱ�� START�� ���ư���.
    INCOMMENT, // "/*"�� ������ ����, '*'�� ������ OUTCOMMENT�� �Ѿ,�׻� ��� IGNORE
    OUTCOMMENT, //INCOMMNET �� '*'�� ������ ����, �ٷ� �̾ '/'�� ������ START, �ٸ� �� ������ INCOMMENT�� ���ư�. �׻�  IGNORE.
    INLESS, //'<'�� ������ ����, '='�� ������ STAY�ϰ� �ƴϸ� BACK�ϰ� START�� ��.
    INGREATER, //'>'�� ������ ����, '='�� ������ STAY�ϰ� �ƴϸ� BACK�ϰ� START�� ��.
    INEQ, //'='�� ������ ����, '='�� ������ STAY�ϰ� �ƴϸ� BACK�ϰ� START�� ��.
    INNE, //'!'�� ������ ����, '='�� ������ STAY�ϰ� �ƴϸ� BACK�ϰ� START�� ��.
}StateType;
typedef enum {
    SAVE, IGNORE, BACK, STAY
    //SAVE�� �ش� ���ڸ� ��ū���ڿ��� �߰��Ѵ�.
    //IGNORE�� �ش� ���ڸ� �����Ѵ�.
    //BACK�� ���� ���ڱ����� �ϳ��� ��ū���� �Ǵ��Ѵ�.
    //STAY�� ���� ���ڱ����� �ϳ��� ��ū���� �Ǵ��Ѵ�.
}LookAhead;
typedef enum {
    ENDFILE,ERROR, ID, NUM,
    /* ����� */
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

FILE* input,* output;//�Է� ���ϰ� ��� ����
StateType state = START; //���� ����
int curLine = 0; //�˻����� ����
typedef struct {
    TokenType type; //���� ��ū ����
    char string[MAX_BUFF_SIZE]; //���� ��ū�� ����
    int index; //���� ��ū ������ �ε���
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
        if (isdigit(c)) //���ڰ� ������ INNUM���� ���� �� SAVE ����
            state = INNUM;
        else if (isalpha(c)) //���ڰ� ������ INID�� ���� �� SAVE ����
            state = INID;
        else {
            switch (c) {//���⼭ case�� �ش�Ǹ� IGNORE
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
                switch (c) { //case�� �ش�Ǹ� ��� STAY
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


    case INNUM: //���ڰ� ������ ����, ���ڰ� ������ SAVE, ���ڰ� �ƴϸ� BACK�ϰ� START�� ���ư�
        if (!isdigit(c)) {
            curToken->type = NUM;
            return BACK;
        }
        return SAVE;
    case INID: //���ڰ� ������ ����, ���ڰ� ������ SAVE, ���ڰ� �ƴϸ� BACK�ϰ� START�� ���ư�
        if (!isalpha(c)) {
            curToken->type = ID;
            return BACK;
        }
        return SAVE;
        break;
    case INDIVIDE: // '/'�� ������ ����, '*'�� ���� IGNORE�ϰ� INCOMMENT�� �Ѿ, �ƴϸ� �׳� BACK���� DIVIDE ��ū�� �ѱ�� START�� ���ư���.
        switch (c) {
        case '*':
            state = INCOMMENT;
            return IGNORE;
        default:
            curToken->type = DIVIDE;
            return BACK;
        }
    case INCOMMENT: // "/*"�� ������ ����, '*'�� ������ OUTCOMMENT�� �Ѿ,�׻� ��� IGNORE
        switch (c) {
        case '*':
            state = OUTCOMMENT;
        default:
            return IGNORE;
        }
    case OUTCOMMENT: //INCOMMNET �� '*'�� ������ ����, �ٷ� �̾ '/'�� ������ START, '*'�� ������ �״�� OUTCOMMENT, �ٸ� �� ������ INCOMMENT�� ���ư�. �׻�  IGNORE.
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
    case INLESS: //'<'�� ������ ����, '='�� ������ STAY�ϰ� �ƴϸ� BACK�ϰ� START�� ��.
        switch (c) {
        case '=':
            curToken->type = LESS_EQUAL;
            return STAY;
        default:
            curToken->type = LESS;
            return BACK;
        }
        break;
    case INGREATER: //'>'�� ������ ����, '='�� ������ STAY�ϰ� �ƴϸ� BACK�ϰ� START�� ��.
        switch (c) {
        case '=':
            curToken->type = GREATER_EQUAL;
            return STAY;
        default:
            curToken->type = GREATER;
            return BACK;
        }
        break;
    case INEQ: //'='�� ������ ����, '='�� ������ STAY�ϰ� �ƴϸ� BACK�ϰ� START�� ��.
        switch (c) {
        case '=':
            curToken->type = EQUAL;
            return STAY;
        default:
            curToken->type = ASSIGN;
            return BACK;
        }
        break;
    case INNE: //'!'�� ������ ����, '='�� ������ STAY�ϰ� �ƴϸ� BACK�ϰ� START�� ��.
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

char lineHold[MAX_BUFF_SIZE] = { 0 }; //�Է����Ϸ� ���� �޾ƿ� ���ڿ� ����
int buff_index = 0;

char getAhead() {
    if (!lineHold[buff_index]) { //���ۿ� �ִ� ���ڸ� ��� Ȯ�� �ߴٸ�
        if (fgets(lineHold, MAX_BUFF_SIZE, input) == NULL) return EOF;  //�������� �޾ƿ�. �̶� ���̻� �޾ƿ� ���� ���ٸ� EOF ��ȯ
        fprintf(OUTPUT, "%2d: %s", ++curLine, lineHold);    //�ش� ���� ȭ�鿡 ���
        if (feof(input)) fprintf(OUTPUT, "\n"); //������ ���� ���� ���� ���. ���͸� ���� �������
        buff_index = 0; //�� ������ ù��° ������ Ȯ���ϵ��� �ʱ�ȭ
    }
    return lineHold[buff_index++]; //���ۿ� �ִ� ���� ���ڸ� ����
}

void backAhead() {
    buff_index--;   //�ش� ���ڸ� ���� ���� ���·� �ǵ���
}

token getToken() {
    token curToken = { ENDFILE,"",0 };
    while (1) {
        char c = getAhead();
        if (c == EOF) {         //���� ���ڰ� ������ ���̶��
            curLine++;          //�����ٷ� �Ѿ��
            return curToken; //ENDFILE ��ū�� ����
        }
        switch (checkAhead(c, &curToken)) {     //lookahead�� �̿��Ͽ� ��� ó���� �� üũ��.
            /*SAVE�� �ش� ���ڸ� ��ū���ڿ��� �߰��Ѵ�.
            IGNORE�� �ش� ���ڸ� �����Ѵ�.
            BACK�� ���� ���ڱ����� �ϳ��� ��ū���� �Ǵ��Ѵ�.
            STAY�� ���� ���ڱ����� �ϳ��� ��ū���� �Ǵ��Ѵ�.*/
        case SAVE: //���� ���ڸ� ��ū�� �߰�
            curToken.string[curToken.index++] = c;
            break;
        case IGNORE:
            // do nothing
            break;
        case BACK:  //���� ���� �������� ��ū ����.
            backAhead();
        case STAY: //���� ���ڱ��� �����Ͽ� ��ū ����.
            curToken.string[curToken.index] = '\0';
            curToken.type = curToken.type == ID ? findKeywords(curToken.string) : curToken.type;
            initState(); //ó�� ���·� �ʱ�ȭ
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