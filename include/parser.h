#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

int parser_process(char *f, void (*emit_value)(void *a, int width));

#endif /* PARSER_H */
