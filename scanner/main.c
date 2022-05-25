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
#if SCAN
        fprintf(OUTPUT, "%2d: %s", ++curLine, lineHold);    //�ش� ���� ȭ�鿡 ���
#endif
#if PARSE
		curLine++;
#endif
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

////////////////////////////////////
//�ļ� ���� ����
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
/// �ļ��� �ʿ��� ��ƿ�� ��
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

//declaration-list �� declaration { declaration }
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

//declaration �� type-specifier ID[("["NUM"]" | "("params")" compund-stmt)];
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

//type-specifier �� int | void
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
//var-declaration �� type-specifier ID ["[" NUM "]"] ;
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

//params �� param-list | void
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

//param-list �� param {, param}
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

//param �� type-specifier ID["[" "]"]
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

//compound-stmt �� "{" local-declarations statement-list "}"
TreeNode* compund_stmt(void){
	TreeNode* t = newStmtNode(CompStmtK);
	match(LBRACE);
	t->child[0] = local_declarations();
	t->child[1] = statement_list();
	match(RBRACE);
	return t;
}

//local-declarations �� {var-declarations}
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

//statement-list �� {statement}
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

//statement �� expression-stmt | compound-stmt | selection-stmt | iteration-stmt | return-stmt
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

//expression-stmt �� [expression] ;
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

//selection-stmt �� if "(" expression ")" statement [else statement]
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

//iteration-stmt �� while "(" expression ")" statement
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

//return-stmt �� return [expression] ;
TreeNode* return_stmt(void){
	TreeNode* t = newStmtNode(RetStmtK);

	match(RETURN);
	if (curToken.type != SEMICOLON && t != NULL)
		t->child[0] = expression();
	match(SEMICOLON);
	return t;
}

//expression �� {var =} simple-expression
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

///�ļ� Ʈ�� ����ϴ� �����
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