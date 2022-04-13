#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#define MAX_BUFF_SIZE 1000
#define DEBUG
#ifdef DEBUG
#define debug(a) a
#define print(a,b) fprintf(output,a,b); fprintf(stdout,a,b)
#else
#define dubug(a)
#define print(a,b) fprintf(output,a,b)
#endif

FILE* input,*output;

void init_input(int argc, char* argv[]) {
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <input filename> <output filename>\n", argv[0]);
        exit(1);
    }
    input = fopen(argv[1], "r");
    output = fopen(argv[2], "w");
    if (input == NULL) {
        print("%s", "No input file");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    init_input(argc,argv);
    int curLine = 1;
    char curTok[MAX_BUFF_SIZE], lookAhead, curLine[MAX_BUFF_SIZE];
    while (!feof(input)) {
        fscanf(input, "%[^\n]", curLine);
        
        print("%c", lookAhead);
    }
    print("\t%d:",curLine);
    print(" %s", "EOF");
}