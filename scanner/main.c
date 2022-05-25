#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#define MAX_BUFF_SIZE 1000
#define TRUE 1
#define FALSE 0
#define SCAN FALSE
#define PARSE TRUE
#define DEBUG FALSE
#if DEBUG
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
#if SCAN
        fprintf(OUTPUT, "%2d: %s", ++curLine, lineHold);    //해당 줄을 화면에 출력
#endif
#if PARSE
		curLine++;
#endif
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

////////////////////////////////////
//파서 구현 시작
///////////////////////////////////
typedef enum { StmtK, ExpK } NodeKind;
typedef enum { CompStmtK, SelStmtK, IterStmtK, RetStmtK, CallK } StmtKind;
typedef enum { VarDeclK, VarArrayDeclK, FuncDeclK, AssignK, OpK, IdK, ConstK } ExpKind;
typedef enum { Void, Integer } ExpType;

#define MAXCHILDREN 3

typedef struct treeNode
{
    struct treeNode* child[MAXCHILDREN];
    struct treeNode* sibling;
    int lineno;
    NodeKind nodekind;
    union { StmtKind stmt; ExpKind exp; } kind;
    union {
        TokenType op;
        int val;
        char* name;
    } attr;
    ExpType type; /* for type checking of exps */
	int isParam;
	int arraysize;
} TreeNode;

char* copyString(char* s)
{
    int n;
    char* t;
    if (s == NULL) return NULL;
    n = strlen(s) + 1;
    t = malloc(n);
    if (t == NULL)
        fprintf(OUTPUT, "Out of memory error\n");
    else strcpy(t, s);
    return t;
}

TreeNode* newStmtNode(StmtKind kind)
{
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL)
		fprintf(OUTPUT, "Out of memory error at line %d\n", curLine);
	else {
		for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = kind;
		t->lineno = curLine;
	}
	return t;
}

/* Function newExpNode creates a new expression
* node for syntax tree construction
*/
TreeNode* newExpNode(ExpKind kind)
{
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL)
		fprintf(OUTPUT, "Out of memory error at line %d\n", curLine);
	else {
		for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = ExpK;
		t->kind.exp = kind;
		t->lineno = curLine;
		t->type = Void;
		t->isParam = FALSE;
	}
	return t;
}


/// <summary>
/// 파서에 필요한 유틸들 끝
/// </summary>

token curToken;

static TreeNode* declaration_list(void);
static TreeNode* declaration(void);
static TreeNode* var_declaration(void);
static ExpType type_spec(void);
static TreeNode* params(void);
static TreeNode* param_list(ExpType type);
static TreeNode* param(ExpType type);
static TreeNode* compund_stmt(void);
static TreeNode* local_declarations(void);
static TreeNode* statement_list(void);
static TreeNode* statement(void);
static TreeNode* expression_stmt(void);
static TreeNode* selection_stmt(void);
static TreeNode* iteration_stmt(void);
static TreeNode* return_stmt(void);
static TreeNode* expression(void);
static TreeNode* simple_expression(TreeNode* f);
static TreeNode* add_expr(TreeNode* f);
static TreeNode* term(TreeNode* f);
static TreeNode* factor(TreeNode* f);
static TreeNode* call(void);
static TreeNode* args(void);
static TreeNode* args_list(void);

static void syntaxError(char* message){
	fprintf(OUTPUT, "\n>>> ");
	fprintf(OUTPUT, "Syntax error at line %d: %s", curLine, message);
}

static void match(TokenType expected){
	if (curToken.type == expected) curToken = getToken();
	else {
		syntaxError("unexpected token -> ");
		printToken(&curToken);
		fprintf(OUTPUT, "      ");
	}
}

//declaration-list → declaration { declaration }
TreeNode* declaration_list(void){
	TreeNode* t = declaration();
	TreeNode* p = t;
	while (curToken.type != ENDFILE){
		TreeNode* q;
		q = declaration();
		if (q != NULL) {
			if (t == NULL) t = p = q;
			else{
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}

//declaration → type-specifier ID[("["NUM"]" | "("params")" compund-stmt)];
TreeNode* declaration(void){
	TreeNode* t=NULL;
	ExpType type;
	char* name;

	type = type_spec();
	curToken = getToken();
	name = copyString(curToken.string);
	match(ID);

	switch (curToken.type)	{
	case SEMICOLON:  //type-specifier ID;
		t = newExpNode(VarDeclK);
		if (t != NULL)
		{
			t->attr.name = name;
			t->type = type;
		}
		match(SEMICOLON);
		break;
	case LSQUARE: //type-specifier ID[NUM];
		t = newExpNode(VarArrayDeclK);
		if (t != NULL)
		{
			t->attr.name = name;
			t->type = type;
		}
		match(LSQUARE);
		if (t != NULL)
			t->arraysize = atoi(curToken.string);
		match(NUM);
		match(RSQUARE);
		match(SEMICOLON);
		break;
	case LPAREN://type-specifier ID(params) compund-stmt;
		t = newExpNode(FuncDeclK);
		if (t != NULL)
		{
			t->attr.name = name;
			t->type = type;
		}
		match(LPAREN);
		if (t != NULL)
			t->child[0] = params();
		match(RPAREN);
		if (t != NULL)
			t->child[1] = compund_stmt();
		break;
	default: syntaxError("unexpected token (decl) -> ");
		printToken(&curToken);
		curToken = getToken();
		break;
	}
	return t;
}

//type-specifier → int | void
ExpType type_spec(void){
	switch (curToken.type) {
	case INT:
		return Integer;
	case VOID:
		return Void;
	default: syntaxError("unexpected token(type_spec) -> ");
		printToken(&curToken);
		return Void;
	}
}
//var-declaration → type-specifier ID ["[" NUM "]"] ;
TreeNode* var_declaration(void){
	TreeNode* t= NULL;
	ExpType type;
	char* name;

	type = type_spec();
	curToken = getToken();
	name = copyString(curToken.string);
	match(ID);
	switch (curToken.type)	{
	case SEMICOLON:
		t = newExpNode(VarDeclK);
		if (t != NULL){
			t->attr.name = name;
			t->type = type;
		}
		match(SEMICOLON);
		break;
	case LSQUARE:
		t = newExpNode(VarArrayDeclK);
		if (t != NULL){
			t->attr.name = name;
			t->type = type;
		}
		match(LSQUARE);
		if (t != NULL)
			t->arraysize = atoi(curToken.string);
		match(NUM);
		match(RSQUARE);
		match(SEMICOLON);
		break;
	default: syntaxError("unexpected token(var_decl) -> ");
		printToken(&curToken);
		curToken = getToken();
		break;
	}
	return t;
}

//params → param-list | void
TreeNode* params(void){
	ExpType type;
	TreeNode* t;

	type = type_spec();
	curToken = getToken();
	if (type == Void && curToken.type == RPAREN)	{
		t = newExpNode(VarDeclK);
		t->isParam = TRUE;
		t->type = Void;
		t->attr.name = "(null)";
	}
	else
		t = param_list(type);
	return t;
}

//param-list → param {, param}
TreeNode* param_list(ExpType type){
	TreeNode* t = param(type);
	TreeNode* p = t;
	TreeNode* q;
	while (curToken.type == COMMA){
		match(COMMA);
		ExpType type = type_spec();
		curToken = getToken();
		q = param(type);
		if (q != NULL) {
			if (t == NULL) t = p = q;
			else /* now p cannot be NULL either */
			{
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}

//param → type-specifier ID["[" "]"]
TreeNode* param(ExpType type){
	TreeNode* t;
	char* name;

	name = copyString(curToken.string);
	match(ID);
	if (curToken.type == LSQUARE)	{
		match(LSQUARE);
		match(RSQUARE);
		t = newExpNode(VarArrayDeclK);
	}
	else
		t = newExpNode(VarDeclK);
	if (t != NULL){
		t->attr.name = name;
		t->type = type;
		t->isParam = TRUE;
	}
	return t;
}

//compound-stmt → "{" local-declarations statement-list "}"
TreeNode* compund_stmt(void){
	TreeNode* t = newStmtNode(CompStmtK);
	match(LBRACE);
	t->child[0] = local_declarations();
	t->child[1] = statement_list();
	match(RBRACE);
	return t;
}

//local-declarations → {var-declarations}
TreeNode* local_declarations(void){
	TreeNode* t = NULL;
	TreeNode* p;

	if (curToken.type == INT || curToken.type == VOID)
		t = var_declaration();
	p = t;
	if (t != NULL){
		while (curToken.type == INT || curToken.type == VOID)	{
			TreeNode* q;
			q = var_declaration();
			if (q != NULL) {
				if (t == NULL) t = p = q;
				else {
					p->sibling = q;
					p = q;
				}
			}
		}
	}
	return t;
}

//statement-list → {statement}
TreeNode* statement_list(void){
	TreeNode* t;
	TreeNode* p;

	if (curToken.type == RBRACE)
		return NULL;
	t = statement();
	p = t;
	while (curToken.type != RBRACE){
		TreeNode* q;
		q = statement();
		if (q != NULL) {
			if (t == NULL) t = p = q;
			else{
				p->sibling = q;
				p = q;
			}
		}
	}

	return t;
}

//statement → expression-stmt | compound-stmt | selection-stmt | iteration-stmt | return-stmt
TreeNode* statement(void){
	TreeNode* t;
	switch (curToken.type)	{
	case LBRACE:
		t = compund_stmt();
		break;
	case IF:
		t = selection_stmt();
		break;
	case WHILE:
		t = iteration_stmt();
		break;
	case RETURN:
		t = return_stmt();
		break;
	case ID:
	case LPAREN:
	case NUM:
	case SEMICOLON:
		t = expression_stmt();
		break;
	default: syntaxError("unexpected token(stmt) -> ");
		printToken(&curToken);
		curToken = getToken();
		return Void;
	}
	return t;
}

//expression-stmt → [expression] ;
TreeNode* expression_stmt(void){
	TreeNode* t = NULL;

	if (curToken.type == SEMICOLON)
		match(SEMICOLON);
	else if (curToken.type != RBRACE)
	{
		t = expression();
		match(SEMICOLON);
	}
	return t;
}

//selection-stmt → if "(" expression ")" statement [else statement]
TreeNode* selection_stmt(void){
	TreeNode* t = newStmtNode(SelStmtK);

	match(IF);
	match(LPAREN);
	if (t != NULL)
		t->child[0] = expression();
	match(RPAREN);
	if (t != NULL)
		t->child[1] = statement();
	if (curToken.type == ELSE)
	{
		match(ELSE);
		if (t != NULL)
			t->child[2] = statement();
	}

	return t;
}

//iteration-stmt → while "(" expression ")" statement
TreeNode* iteration_stmt(void){
	TreeNode* t = newStmtNode(IterStmtK);

	match(WHILE);
	match(LPAREN);
	if (t != NULL)
		t->child[0] = expression();
	match(RPAREN);
	if (t != NULL)
		t->child[1] = statement();
	return t;
}

//return-stmt → return [expression] ;
TreeNode* return_stmt(void){
	TreeNode* t = newStmtNode(RetStmtK);

	match(RETURN);
	if (curToken.type != SEMICOLON && t != NULL)
		t->child[0] = expression();
	match(SEMICOLON);
	return t;
}

//expression → {var =} simple-expression
TreeNode* expression(void){
	TreeNode* t = NULL;
	TreeNode* q = NULL;
	int flag = FALSE;

	if (curToken.type == ID){
		q = call();
		flag = TRUE;
	}

	if (flag == TRUE && curToken.type == ASSIGN)	{
		if (q != NULL && q->nodekind == ExpK && q->kind.exp == IdK)	{
			match(ASSIGN);
			t = newExpNode(AssignK);
			if (t != NULL){
				t->child[0] = q;
				t->child[1] = expression();
			}
		}
		else{
			syntaxError("wrong lvalue\n");
			curToken = getToken();
		}
	}
	else
		t = simple_expression(q);
	return t;
}

TreeNode* simple_expression(TreeNode* f){
	TreeNode* t, * q;
	TokenType oper;
	q = add_expr(f);
	if (curToken.type == LESS || curToken.type == LESS_EQUAL || curToken.type == GREATER_EQUAL || curToken.type == GREATER || curToken.type == EQUAL || curToken.type == NOT_EQUAL)
	{
		oper = curToken.type;
		match(curToken.type);
		t = newExpNode(OpK);
		if (t != NULL)
		{
			t->child[0] = q;
			t->child[1] = add_expr(NULL);
			t->attr.op = oper;
		}
	}
	else
		t = q;
	return t;
}

TreeNode* add_expr(TreeNode* f){
	TreeNode* t;
	TreeNode* q;

	t = term(f);
	if (t != NULL){
		while (curToken.type == PLUS || curToken.type == MINUS){
			q = newExpNode(OpK);
			if (q != NULL) {
				q->child[0] = t;
				q->attr.op = curToken.type;
				t = q;
				match(curToken.type);
				t->child[1] = term(NULL);

			}
		}
	}
	return t;
}

TreeNode* term(TreeNode* f){
	TreeNode* t;
	TreeNode* q;

	t = factor(f);
	if (t != NULL){
		while (curToken.type == TIMES || curToken.type == DIVIDE)	{
			q = newExpNode(OpK);
			if (q != NULL) {
				q->child[0] = t;
				q->attr.op = curToken.type;
				t = q;
				match(curToken.type);
				t->child[1] = factor(NULL);

			}
		}
	}
	return t;
}

TreeNode* factor(TreeNode* f){
	TreeNode* t;

	if (f != NULL)
		return f;

	switch (curToken.type)	{
	case LPAREN:
		match(LPAREN);
		t = expression();
		match(RPAREN);
		break;
	case ID:
		t = call();
		break;
	case NUM:
		t = newExpNode(ConstK);
		if (t != NULL){
			t->attr.val = atoi(curToken.string);
			t->type = Integer;
		}
		match(NUM);
		break;
	default: syntaxError("unexpected token(factor) -> ");
		printToken(&curToken);
		curToken = getToken();
		return Void;
	}
	return t;
}

TreeNode* call(void){
	TreeNode* t;
	char* name=NULL;

	if (curToken.type == ID)
		name = copyString(curToken.string);
	match(ID);

	if (curToken.type == LPAREN)
	{
		match(LPAREN);
		t = newStmtNode(CallK);
		if (t != NULL)
		{
			t->attr.name = name;
			t->child[0] = args();
		}
		match(RPAREN);
	}
	else if (curToken.type == LSQUARE){
		t = newExpNode(IdK);
		if (t != NULL){
			t->attr.name = name;
			t->type = Integer;
			match(LSQUARE);
			t->child[0] = expression();
			match(RSQUARE);
		}
	}
	else{
		t = newExpNode(IdK);
		if (t != NULL){
			t->attr.name = name;
			t->type = Integer;
		}
	}
	return t;
}

TreeNode* args(void){
	if (curToken.type == RPAREN)
		return NULL;
	else
		return args_list();
}

TreeNode* args_list(void){
	TreeNode* t;
	TreeNode* p;

	t = expression();
	p = t;
	if (t != NULL){
		while (curToken.type == COMMA){
			match(COMMA);
			TreeNode* q = expression();
			if (q != NULL) {
				if (t == NULL) t = p = q;
				else /* now p cannot be NULL either */
				{
					p->sibling = q;
					p = q;
				}
			}
		}
	}
	return t;
}

char* typeName(ExpType type){
	switch (type){
	case Integer: return "int"; break;
	case Void:    return "void"; break;
	default:      return "wrong type";
	}
}

TreeNode* parse(void){
    TreeNode* t;
    curToken = getToken();
    t = declaration_list();
    if (curToken.type != ENDFILE)
        syntaxError("Code ends before file\n");
    return t;
}

///파서 트리 출력하는 내용들
static indentno = 0;

#define INDENT indentno+=2
#define UNINDENT indentno-=2

static void printSpaces(void){
    int i;
    for (i = 0; i < indentno; i++)
        fprintf(OUTPUT, " ");
}

void printTree(TreeNode* tree){
	int i;
	INDENT;
	while (tree != NULL) {
		i = 0;
		printSpaces();
		if (tree->nodekind == StmtK)
		{
			switch (tree->kind.stmt) {
			case CompStmtK:
				fprintf(OUTPUT, "Compound Statement :\n");
				break;
			case SelStmtK:
				if (tree->child[2] != NULL)
					fprintf(OUTPUT, "If with else\n");
				else
					fprintf(OUTPUT, "If without else\n");
				break;
			case IterStmtK:
				fprintf(OUTPUT, "While\n");
				break;
			case RetStmtK:
					fprintf(OUTPUT, "Return\n");
				break;
			case CallK:
				if (tree->child[0] != NULL)
					fprintf(OUTPUT, "Call %s, with arguments\n", tree->attr.name);
				else
					fprintf(OUTPUT, "Call %s\n", tree->attr.name);
				break;
			default:
				fprintf(OUTPUT, "Unknown ExpNode kind\n");
				break;
			}
		}
		else if (tree->nodekind == ExpK)
		{
			switch (tree->kind.exp) {
			case VarDeclK:
				if (tree->isParam == TRUE)
					fprintf(OUTPUT, "Single Parameter, name : %s, type : %s\n", tree->attr.name, typeName(tree->type));
				else
					fprintf(OUTPUT, "Var Declaration, name : %s, type : %s\n", tree->attr.name, typeName(tree->type));
				break;
			case VarArrayDeclK:
				if (tree->isParam == TRUE)
					fprintf(OUTPUT, "Array Parameter, name : %s, type : %s\n", tree->attr.name, typeName(tree->type));
				else
					fprintf(OUTPUT, "Array Var Declaration, name : %s, type : %s, size : %d\n", tree->attr.name, typeName(tree->type), tree->arraysize);
				break;
			case FuncDeclK:
				fprintf(OUTPUT, "Function Declaration, name : %s, type : %s\n", tree->attr.name, typeName(tree->type));
				break;
			case AssignK:
				fprintf(OUTPUT, "Assign to: %s\n",tree->child[0]->attr.name);
				i++;
				break;
			case OpK:
				fprintf(OUTPUT, "Op : ");
				fprintf(OUTPUT, "%s\n", specialSymbols[tree->attr.op - FIRST_SPECIAL_SYMBOL]);
				break;
			case IdK:
				fprintf(OUTPUT, "Id : %s\n", tree->attr.name);
				break;
			case ConstK:
				fprintf(OUTPUT, "Const : %d\n", tree->attr.val);
				break;
			default:
				fprintf(OUTPUT, "Unknown ExpNode kind\n");
				break;
			}
		}
		else fprintf(OUTPUT, "Unknown node kind\n");
		for (; i < MAXCHILDREN; i++)
			printTree(tree->child[i]);
		tree = tree->sibling;
	}
	UNINDENT;
}

int main(int argc, char* argv[]) {
    initInput(argc,argv);
#if SCAN
    do {
        curToken = getToken();
        printToken(&curToken);
    } while (curToken.type != ENDFILE);
#endif
#if PARSE
    printTree(parse());
#endif
    return 0;
}