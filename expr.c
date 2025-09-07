#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include "debug.h"
#include "expr.h"
#include "tokens.h"

#define EXPR_STACK_INIT_SIZE	16

struct expr_stack {
	int *stack;	
	int numitems;
	int allocsize;
};

struct expr {
	struct expr_stack values_stack;
	struct expr_stack ops_stack;
};

struct expr_op {
	const char *sym;
	int opcode;
	int precedence;
};

static struct expr_op expr_ops[] = {
	{"~", TOK_L_NEG, 7},
	{"*", TOK_OP_MUL, 6},
	{"/", TOK_OP_DIV, 6},
	{"%", TOK_OP_MOD, 6},
	{"+", TOK_OP_ADD, 5},
	{"-", TOK_OP_SUB, 5},
	{"<<", TOK_L_LSHIFT, 4},
	{">>", TOK_L_RSHIFT, 4},
	{"&", TOK_L_AND, 3},
	{"^", TOK_L_XOR, 2},
	{"|", TOK_L_OR, 1},
	{"(", TOK_OPAREN, -1},
	{")", TOK_CPAREN, 0},
	{",", TOK_SEP, -3},
	{NULL, 0, 0}
};

static int expr_stack_get_prev_token(struct expr_stack *expr_stack)
{
	return expr_stack->stack[expr_stack->numitems - 1];
}

static int expr_op_get_precedence(int op)
{
	int precedence = 0;
	struct expr_op *expr_op = expr_ops;

	while (expr_op) {
		if (expr_op->opcode == op) {
			precedence = expr_op->precedence;
			break;
		}
		
		expr_op++;
	}

	return precedence;
}

static const char *expr_op_get_symbol(int op)
{
	const char *sym = NULL;
	struct expr_op *expr_op = expr_ops;

	while (expr_op) {
		if (expr_op->opcode == op) {
			sym = expr_op->sym;
			break;
		}
		expr_op++;
	}

	return sym;
}

static void expr_dump_stack(struct expr_stack *expr_stack, const char *name, int print_symbol)
{
	verbose_printf("%s stack: ", name);
	for (int i = 0; i < expr_stack->numitems; i++) {
		if (print_symbol)
			verbose_printf("%s ", expr_op_get_symbol(expr_stack->stack[i]));
		else
			verbose_printf("%d ", expr_stack->stack[i]);
	}
	verbose_printf("\n");
}

static int expr_is_stack_empty(struct expr_stack *expr_stack)
{
	return !expr_stack->numitems;
}

static int expr_pop_stack(struct expr_stack *expr_stack) 
{
	int op;

	op = expr_stack->stack[expr_stack->numitems - 1];
	expr_stack->numitems--;

	return op;
}

static int expr_push_stack(struct expr_stack *expr_stack, int token) 
{
	int ret = 0;
	int newsize;

	if (expr_stack->numitems == expr_stack->allocsize) {
		newsize = expr_stack->allocsize * 2;
		expr_stack->stack = realloc(expr_stack->stack, newsize * sizeof(int));
		if (!expr_stack->stack) {
			error_printf("cannot expand stack\n");
			ret = -ENOMEM;
			goto err;
		}

		expr_stack->allocsize = newsize;
	}

	expr_stack->stack[expr_stack->numitems] = token;
	expr_stack->numitems++;

err:
	return ret;
}

static void expr_run(struct expr *expr, int op)
{
	int val;
	int a, b;

	if (op == TOK_L_NEG) {
		a = expr_pop_stack(&expr->values_stack);
	} else if (op != TOK_CPAREN) {
		b = expr_pop_stack(&expr->values_stack);
		a = expr_pop_stack(&expr->values_stack);
	} else {
		return;
	}

	switch(op) {
	case TOK_L_NEG:
		val = ~a;
		verbose_printf("#%d#\n", val);
		break;
	case TOK_OP_MUL:
		val = a * b;
		verbose_printf("#%d * %d = %d#\n", a, b, a * b);
		break;
	case TOK_OP_DIV:
		val = a / b;
		verbose_printf("#%d / %d = %d#\n", a, b, a / b);
		break;
	case TOK_OP_MOD:
		val = a % b;
		verbose_printf("#%d %% %d = %d#\n", a, b, a % b);
		break;
	case TOK_OP_ADD:
		val = a + b;
		verbose_printf("#%d + %d = %d#\n", a, b, a + b);
		break;
	case TOK_OP_SUB:
		val = a - b;
		verbose_printf("#%d - %d = %d#\n", a, b, a - b);
		break;
	case TOK_L_LSHIFT:
		val = a << b;
		verbose_printf("#%d << %d = %d#\n", a, b, a << b);
		break;
	case TOK_L_RSHIFT:
		val = a >> b;
		verbose_printf("#%d >> %d = %d#\n", a, b, a >> b);
		break;
	case TOK_L_AND:
		val = a & b;
		verbose_printf("#%d & %d = %d#\n", a, b, a & b);
		break;
	case TOK_L_XOR:
		val = a ^ b;
		verbose_printf("#%d ^ %d = %d#\n", a, b, a ^ b);
		break;
	case TOK_L_OR:
		val = a | b;
		verbose_printf("#%d | %d = %d#\n", a, b, a | b);
		break;
	}

	expr_push_stack(&expr->values_stack, val);
}

struct expr *expr_init(void)
{
	struct expr *expr;

	expr = malloc(sizeof(*expr));
	if (!expr) {
		error_printf("failed to allocate expr struct\n");
		goto err;
	}

	expr->values_stack.numitems = 0; 
	expr->values_stack.allocsize = EXPR_STACK_INIT_SIZE; 
	expr->values_stack.stack = malloc(EXPR_STACK_INIT_SIZE * sizeof(int));
	if (!expr->values_stack.stack) {
		error_printf("failed to allocate expr values stack\n");
		goto err_free_expr;
	}

	expr->ops_stack.numitems = 0; 
	expr->ops_stack.allocsize = EXPR_STACK_INIT_SIZE;
	expr->ops_stack.stack = malloc(EXPR_STACK_INIT_SIZE * sizeof(int));
	if (!expr->ops_stack.stack) {
		error_printf("failed to allocate expr op stack\n");
		goto err_free_val_stack;
	}


	return expr;

err_free_val_stack:
	free(expr->values_stack.stack);
err_free_expr:
	free(expr);
err:
	return NULL;
}

void expr_deinit(struct expr *expr)
{
	free(expr->ops_stack.stack);
	free(expr->values_stack.stack);
}

int expr_eval(struct expr *expr)
{
	return expr_pop_stack(&expr->values_stack);
}

int expr_push_operator(struct expr *expr, int op)
{
	int ret = 0;
	int precedence;
	int prev_op;
	int prev_precedence;

	precedence = expr_op_get_precedence(op);

	do {
		if (expr_is_stack_empty(&expr->ops_stack))
			goto out;

		expr_dump_stack(&expr->values_stack, "values", 0);
		expr_dump_stack(&expr->ops_stack, "operators", 1);

		prev_op = expr_stack_get_prev_token(&expr->ops_stack);
		prev_precedence = expr_op_get_precedence(prev_op);

		verbose_printf("%s >= %s ?\n", expr_op_get_symbol(prev_op), expr_op_get_symbol(op));
		if (op == TOK_OPAREN) {
			goto out;
		} else if (prev_op == TOK_OPAREN && op == TOK_CPAREN) {
			expr_pop_stack(&expr->ops_stack);
		} else if (prev_precedence >= precedence) {
			expr_pop_stack(&expr->ops_stack);
			expr_run(expr, prev_op);
		}
	} while (prev_precedence >= precedence);

out:
	if (op != TOK_SEP)
		ret = expr_push_stack(&expr->ops_stack, op);

	return ret;
}

int expr_push_value(struct expr *expr, int value)
{
	int ret = 0;

	ret = expr_push_stack(&expr->values_stack, value);

	return ret;
}
