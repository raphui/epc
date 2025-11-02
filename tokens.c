#include "tokens.h"


int is_token_an_operator(int token)
{
	int ret = 0;

	switch (token) {
	case TOK_L_NEG:
	case TOK_OP_MUL:
	case TOK_OP_DIV:
	case TOK_OP_MOD:
	case TOK_OP_ADD:
	case TOK_OP_SUB:
	case TOK_L_LSHIFT:
	case TOK_L_RSHIFT:
	case TOK_L_AND:
	case TOK_L_XOR:
	case TOK_L_OR:
		ret = 1;
		break;
	}

	return ret;
}
