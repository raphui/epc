#ifndef TOKENS_H
#define TOKENS_H

#define TOK_TYPE(tok)		(#tok)

#define TOK_EOF			0x1
#define TOK_SEP			0x2
#define TOK_LF			0x3
#define TOK_NUMBER		0x4
#define TOK_HEX_SEP		0x5

#define TOK_L_OR		0x10
#define TOK_L_AND		0x11
#define TOK_L_XOR		0x12
#define TOK_L_LSHIFT		0x13
#define TOK_L_RSHIFT		0x14
#define TOK_L_NEG		0x15

#define TOK_OPAREN		0x20
#define TOK_CPAREN		0x21
#define TOK_OBRACKET		0x22
#define TOK_CBRACKET		0x23
#define TOK_ONODE		0x24
#define TOK_CNODE		0x25
#define TOK_ASSIGN		0x26
#define TOK_ENDASSIGN		0x27

#define TOK_OP_ADD		0x100
#define TOK_OP_SUB		0x101
#define TOK_OP_MUL		0x102
#define TOK_OP_DIV		0x103
#define TOK_OP_MOD		0x104

#define TOK_TYPE_BYTE		0x200
#define TOK_TYPE_SHORT		0x201
#define TOK_TYPE_INT		0x202
#define TOK_TYPE_FLOAT		0x203

#endif /* TOKENS_H */
