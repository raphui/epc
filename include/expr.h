#ifndef EXPR_H
#define EXPR_H

struct expr;

struct expr *expr_init(void);
void expr_deinit(struct expr *expr);
int expr_eval(struct expr *expr);
int expr_push_operator(struct expr *expr, int op);
int expr_push_value(struct expr *expr, int value);

#endif /* EXPR_H */
