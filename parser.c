#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "debug.h"
#include "parser.h"
#include "expr.h"
#include "tokens.h"

struct parser {
	int node_depth;
	int type;
	int prev_type;
	int in_assign;
	int width;
	int cur_line;
	int cur_col;
	int error;
	char *f;
	uint8_t *start_tok;
	uint8_t *end_tok;
	struct expr *expr;
};

static void parser_syntax_error(int line, int col)
{
	error_printf("syntax error: %d:%d\n", line, col);
}

int parser_process(char *f, void (*emit_value)(void *a, int width))
{
	int val;
	int base;
	uint32_t mask;
	char *end;
	int ret = 0;
	struct parser *parser;

	parser = malloc(sizeof(*parser));
	if (!parser) {
		error_printf("cannot alloc parser struct\n");
		ret = -ENOMEM;
		goto err;
	}

	memset(parser, 0, sizeof(*parser));

	parser->expr = expr_init();
	if (!parser->expr) {
		error_printf("cannot init expr engine\n");
		ret = -ENOMEM;
		goto err_free_parser;
	}

	parser->f = f;
	parser->cur_line = 1;
	parser->node_depth = -1;
	parser->prev_type = -1;

	while (*f != '\0') {
		verbose_printf("%c", *f);

		if (*f == ' ' || *f == '\t') {
			f++;
			parser->cur_col++;
			continue;
		} else if (*f == '\n') {
			verbose_printf("\n");
			parser->cur_line++;
			parser->cur_col = 1;
			f++;
			continue;
		}

		switch (*f) {
		case '{':
			parser->node_depth++;
			parser->type = TOK_ONODE;

			verbose_printf(TOK_TYPE(TOK_ONODE));
			break;
		case '}':
			parser->node_depth--;
			parser->type = TOK_CNODE;

			verbose_printf(TOK_TYPE(TOK_CNODE));
			break;
		case '(':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}

			parser->type = TOK_OPAREN;

			ret = expr_push_operator(parser->expr, parser->type);
			if (ret < 0)
				goto err_free_expr;

			verbose_printf(TOK_TYPE(TOK_OPAREN));
			break;
		case ')':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}

			parser->type = TOK_CPAREN;
			ret = expr_push_operator(parser->expr, parser->type);
			if (ret < 0)
				goto err_free_expr;

			verbose_printf(TOK_TYPE(TOK_CPAREN));
			break;
		case '&':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}

			parser->type = TOK_L_AND;
			ret = expr_push_operator(parser->expr, parser->type);
			if (ret < 0)
				goto err_free_expr;

			verbose_printf(TOK_TYPE(TOK_L_AND));
			break;
		case '|':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}

			parser->type = TOK_L_OR;
			ret = expr_push_operator(parser->expr, parser->type);
			if (ret < 0)
				goto err_free_expr;

			verbose_printf(TOK_TYPE(TOK_L_OR));
			break;
		case '^':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}

			parser->type = TOK_L_XOR;
			ret = expr_push_operator(parser->expr, parser->type);
			if (ret < 0)
				goto err_free_expr;

			verbose_printf(TOK_TYPE(TOK_L_XOR));
			break;
		case '~':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}

			parser->type = TOK_L_NEG;
			ret = expr_push_operator(parser->expr, parser->type);
			if (ret < 0)
				goto err_free_expr;

			verbose_printf(TOK_TYPE(TOK_L_NEG));
			break;
		case '<':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			} else if (parser->prev_type == TOK_OBRACKET) {
				parser->type = TOK_L_LSHIFT;
				ret = expr_push_operator(parser->expr, parser->type);
				if (ret < 0)
					goto err_free_expr;

				verbose_printf(TOK_TYPE(TOK_L_LSHIFT));
			} else {
				parser->type = TOK_OBRACKET;	

				verbose_printf(TOK_TYPE(TOK_OBRACKET));
			}
			break;
		case '>':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			} else if (parser->prev_type == TOK_CBRACKET) {
				parser->type = TOK_L_RSHIFT;
				ret = expr_push_operator(parser->expr, parser->type);
				if (ret < 0)
					goto err_free_expr;

				verbose_printf(TOK_TYPE(TOK_L_RSHIFT));
			} else {
				parser->type = TOK_CBRACKET;

				verbose_printf(TOK_TYPE(TOK_CBRACKET));
			}

			break;
		case '+':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}

			parser->type = TOK_OP_ADD;
			ret = expr_push_operator(parser->expr, parser->type);
			if (ret < 0)
				goto err_free_expr;

			verbose_printf(TOK_TYPE(TOK_OP_ADD));
			break;
		case '-':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}

			parser->type = TOK_OP_SUB;
			ret = expr_push_operator(parser->expr, parser->type);
			if (ret < 0)
				goto err_free_expr;

			verbose_printf(TOK_TYPE(TOK_OP_SUB));
			break;
		case '/':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}

			parser->type = TOK_OP_DIV;
			ret = expr_push_operator(parser->expr, parser->type);
			if (ret < 0)
				goto err_free_expr;

			verbose_printf(TOK_TYPE(TOK_OP_DIV));
			break;
		case '*':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}

			parser->type = TOK_OP_MUL;
			ret = expr_push_operator(parser->expr, parser->type);
			if (ret < 0)
				goto err_free_expr;

			verbose_printf(TOK_TYPE(TOK_OP_MUL));
			break;
		case '%':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}

			parser->type = TOK_OP_MOD;
			ret = expr_push_operator(parser->expr, parser->type);
			if (ret < 0)
				goto err_free_expr;

			verbose_printf(TOK_TYPE(TOK_OP_MOD));
			break;
		case '=':
			if (parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}
			
			parser->in_assign = 1;
			parser->type = TOK_ASSIGN;
			verbose_printf(TOK_TYPE(TOK_ASSIGN));
			break;
		case ';':
			if (parser->prev_type == TOK_CNODE) {
				parser->node_depth--;
			} else if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			} else {
				parser->in_assign = 0;
				parser->type = TOK_ENDASSIGN;
			}

			verbose_printf(TOK_TYPE(TOK_ENDASSIGN));
			break;
		case ',':
			if (!parser->in_assign) {
				parser->error = 1;
				goto err_parsing;
			}

			parser->type = TOK_SEP;
			ret = expr_push_operator(parser->expr, parser->type);
			if (ret < 0)
				goto err_free_expr;

			verbose_printf(TOK_TYPE(TOK_SEP));
			break;
		case 'u':
			if (parser->in_assign && parser->prev_type == TOK_SEP) {
				val = strtol((const char *)(f + 1), &end, 10);
				if (!val) {
					parser->error = 1;
					goto err_parsing;
				}

				if (val == 8) {
					parser->type = TOK_TYPE_BYTE;
					verbose_printf(TOK_TYPE(TOK_TYPE_BYTE));
					mask = 0xFF;
				} else if (val == 16) {
					parser->type = TOK_TYPE_SHORT;
					verbose_printf(TOK_TYPE(TOK_TYPE_SHORT));
					mask = 0xFFFF;
				} else if (val == 32) {
					parser->type = TOK_TYPE_INT;
					verbose_printf(TOK_TYPE(TOK_TYPE_INT));
					mask = 0xFFFFFFFF;
				} else {
					parser->error = 1;
					goto err_parsing;
				}

				parser->width = val / 8;

				val = expr_eval(parser->expr);
				val &= mask;

				emit_value(&val, parser->width);

				f = end - 1;
			}
			break;
		default:
			if (parser->in_assign) {
				if (*f == '0')
					base = 16;
				else
					base = 10;

				val = strtol((const char *)f, &end, base);
				if (!val) {
					parser->error = 1;
					goto err_parsing;
				}

				expr_push_value(parser->expr, val);

				if (base == 16)
					verbose_printf("(0x%x)", val);
				else
					verbose_printf("(%d)", val);

				f = end - 1;
			}
			break;
		}
		
		f++;
		parser->cur_col++;
		parser->prev_type = parser->type;
	}

err_parsing:
	if (parser->error) {
		parser_syntax_error(parser->cur_line, parser->cur_col);
		ret = -EINVAL;
	}
err_free_expr:
	expr_deinit(parser->expr);
	free(parser->expr);
err_free_parser:
	free(parser);
err:
	return ret;
}
