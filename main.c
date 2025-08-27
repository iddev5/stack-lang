#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define STACK_INIT 255
#define BIND_INIT 64

struct value;

// TODO: better hashing function/strategy
unsigned int hash(char *string, int cap) {
	unsigned int code = 0;
	for (code = 0; *string != '\0'; string++) {
		code = *string + 32 * code;
	}

	return code % cap;
}

enum { BNONE, BNATIVE, BFUNC };

struct bind_value {
	int type;
	union {
		void (*native)(struct value *, int *);
		char *func;
	};
};

#define MK_NATIVE(nat) (struct bind_value){ .type = BNATIVE, .native = nat }
#define MK_NONE() (struct bind_value){ .type = BNONE, .native = NULL }

// TODO
struct bind_value_kv {
	char *key;
	struct bind_value value;
};

struct bind_map {
	struct bind_value *kvs;
	int cap;
};

struct bind_value bind(struct bind_map* map, char *key, struct bind_value value) {
	if (map->kvs == NULL) {
		map->cap = BIND_INIT;
		map->kvs = (struct bind_value *)malloc(sizeof(struct bind_value) * map->cap);
	}

	int hashed_key = hash(key, map->cap);

	if (value.type != BNONE)
		map->kvs[hashed_key] = value;
	else
		value = map->kvs[hashed_key];

	return value;
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

void op_dup(struct value *stack, int *top) {
	struct value a = stack[*top];
	struct value duped_a = a;
	if (a.type == VSTR)
		duped_a.v_str = strdup(a.v_str);
	stack[++*top] = duped_a;
}

void interp(char *source, int n) {
	struct value stack[STACK_INIT];
	int top = -1;
	char *tok, *rest = source;

	struct bind_map map = {0};
	bind(&map, "+", MK_NATIVE(op_add));
	bind(&map, "-", MK_NATIVE(op_sub));
	bind(&map, "*", MK_NATIVE(op_mul));
	bind(&map, "/", MK_NATIVE(op_div));

	bind(&map, "print", MK_NATIVE(op_print));
	bind(&map, "dup", MK_NATIVE(op_dup));

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
    	} else if (strcmp(tok, "def") == 0) {
    		char *name = strtok_r(rest, " ", &rest);
    		printf("fn name: %s\nfn content: ", name);

    		char *end = strstr(rest, ";;");
			while (rest != end + 2) {
				putchar(*rest++);
			}
    		
    	} else {
    		struct bind_value bv = bind(&map, tok, MK_NONE());

    		switch (bv.type) {
    			case BNATIVE:
    				bv.native(stack, &top);
    				break;
    			default:
    				int blank = 0;
    				while (isspace(tok[blank++]));

    				if (blank < 1)
    					printf("unimplemented: '%s'\n", tok);
    				break;
    		}
    	}
    }
}

int main() {
    char source[255];
    fread(source, 255, sizeof(char), stdin);

    printf("contents: %s\n", source);

    // printf("%d", n);
    interp(source, 255);
}
