/*
 * tinyexpr.h
 *
 * Project: tinyexpr
 * Author: codeplea
 * Source: https://github.com/codeplea/tinyexpr
 * License: zlib
 */

#ifndef TINYEXPR_H
#define TINYEXPR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct te_expr te_expr;

/* Bound variable type */
#define TE_VARIABLE 7
#define TE_FUNCTION0 1
#define TE_FUNCTION1 2
#define TE_FUNCTION2 3
#define TE_FUNCTION3 4
#define TE_FUNCTION4 5
#define TE_PURE 6

typedef struct {
    const char *name;
    const void *address;
    int type;
    void *context;
} te_variable;

/* Compiles an expression. */
te_expr *te_compile(const char *expression, const te_variable *variables, int var_count, int *error);

/* Evaluate an expression. */
double te_eval(const te_expr *n);

/* Free the expression. */
void te_free(te_expr *n);

/* Interp: compiles and evaluates in one go. */
double te_interp(const char *expression, int *error);

#ifdef __cplusplus
}
#endif

#endif /* TINYEXPR_H */
