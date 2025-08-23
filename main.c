#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define STACK_INIT 255

unsigned int hash(char *string, int cap) {
	unsigned int code = 0;
	for (code = 0; *string != '\0'; string++) {
		code = *string + 31 * code;
	}

	return code % cap;
}

enum { VINT, VFLOAT, VSTR };

struct value {
	int type;
	union {
		int v_int;
		float v_float;
		char *v_str;
	};
};

#define M_CONCAT(a, b) a ## b

#define BIN_OP(name, sym) \
void M_CONCAT(op_, name)(struct value *stack, int *top) { \
    struct value b = stack[(*top)--], a = stack[(*top)--]; \
	if (a.type == VFLOAT || b.type == VFLOAT) { \
		float res = 0; \
		if (a.type != VFLOAT) res = a.v_int sym b.v_float; \
		else if (b.type != VFLOAT) res = a.v_float sym b.v_int; \
		else res = a.v_float sym b.v_float; \
		stack[++*top] = (struct value){ .type = VFLOAT, .v_float = res }; \
	} else\
		stack[++*top] = (struct value){ .type = VINT, .v_int = a.v_int sym b.v_int }; \
} \

BIN_OP(add, +);
BIN_OP(sub, -);
BIN_OP(mul, *);
BIN_OP(div, /);

void op_print(struct value *stack, int *top) {
	struct value a = stack[*top];

	switch (a.type) {
		case VINT:
			printf("%d", a.v_int);
			break;
		case VFLOAT:
			printf("%f", a.v_float);
			break;
		case VSTR:
			printf("%s", a.v_str);
			break;
	}
}

void interp(char *source, int n) {
	struct value stack[STACK_INIT];
	int top = -1;
	char *tok, *rest = source;

    while (tok = strtok_r(rest, " ", &rest)) {
    	if (isdigit(*tok)) {
    		int i = 1, len = strlen(tok), is_float = 0; 
    		while (i < len && (isdigit(tok[i++]) || tok[i-1] == '.'))
    			if (tok[i-1] == '.') is_float++;

    		switch (is_float) {
    			case 0: {
					int num = atoi(tok);
		    		stack[++top] = (struct value){ .type = VINT, .v_int = num };
					break;
				}
    			case 1: {
					float num = atof(tok);
					stack[++top] = (struct value){ .type = VFLOAT, .v_float = num };
					break;
				}
    			default:
    				printf("error: invalid token\n");
    				break;
    		}
    	} else if (*tok == '"') {
    		int i = 1;
    		while (tok[i++] != '"');

			char *str = strndup(tok + 1, i - 2);
    		stack[++top] = (struct value){ .type = VSTR, .v_str = str };
    	} else if (strcmp(tok, "+") == 0) op_add(stack, &top);
    	else if (strcmp(tok, "-") == 0) op_sub(stack, &top);
		else if (strcmp(tok, "*") == 0) op_mul(stack, &top);
		else if (strcmp(tok, "/") == 0) op_div(stack, &top);
    	else if (strcmp(tok, "print") == 0)
    		op_print(stack, &top);
    }
}

int main() {
    char source[255];
    fread(source, 255, sizeof(char), stdin);

    printf("contents: %s\n", source);

    // printf("%d", n);
    interp(source, 255);
}
