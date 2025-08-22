#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define STACK_INIT 255

void interp(char *source, int n) {
	int stack[STACK_INIT], top = 0;
	char *tok, *rest = source;

    while (tok = strtok_r(rest, " ", &rest)) {
    	// printf("token: %s\n", tok);

    	if (isdigit(*tok)) {
    		int i = 1, len = strlen(tok); 
    		while (i < len && isdigit(tok[i++]));

			int num = atoi(tok);
    		stack[top++] = num;
    	} else if (strcmp(tok, "+") == 0) {
    		int a = stack[--top], b = stack[--top];
    		stack[++top] = a + b;
    	} else if (strcmp(tok, ".") == 0)
    		printf("%d", stack[top]);
    }
}

int main() {
    char source[255];
    fread(source, 255, sizeof(char), stdin);

    printf("contents: %s\n", source);

    // printf("%d", n);
    interp(source, 255);
}
