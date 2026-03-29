/*
 * tinyexpr.c
 *
 * Project: tinyexpr
 * Author: codeplea
 * Source: https://github.com/codeplea/tinyexpr
 * License: zlib
 */

#include "tinyexpr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define TE_MAX_ARGS 4

struct te_expr {
    int type;
    union {
        double value;
        const double *bound;
        const void *function;
    } item;
    void *context;
    struct te_expr *param[TE_MAX_ARGS];
};

/* Built-in functions */
static double _add(double a, double b) { return a + b; }
static double _sub(double a, double b) { return a - b; }
static double _mul(double a, double b) { return a * b; }
static double _div(double a, double b) { return a / b; }
static double _mod(double a, double b) { return fmod(a, b); }
static double _pow(double a, double b) { return pow(a, b); }
static double _sqrt(double a) { return sqrt(a); }
static double _sin(double a) { return sin(a); }
static double _cos(double a) { return cos(a); }
static double _tan(double a) { return tan(a); }
static double _asin(double a) { return asin(a); }
static double _acos(double a) { return acos(a); }
static double _atan(double a) { return atan(a); }
static double _sinh(double a) { return sinh(a); }
static double _cosh(double a) { return cosh(a); }
static double _tanh(double a) { return tanh(a); }
static double _exp(double a) { return exp(a); }
static double _log(double a) { return log(a); }
static double _log10(double a) { return log10(a); }
static double _abs(double a) { return fabs(a); }
static double _ceil(double a) { return ceil(a); }
static double _floor(double a) { return floor(a); }
static double _round(double a) { return round(a); }

/* Logic */
static double _equal(double a, double b) { return a == b; }
static double _not_equal(double a, double b) { return a != b; }
static double _less(double a, double b) { return a < b; }
static double _less_equal(double a, double b) { return a <= b; }
static double _greater(double a, double b) { return a > b; }
static double _greater_equal(double a, double b) { return a >= b; }
static double _and(double a, double b) { return (a != 0) && (b != 0); }
static double _or(double a, double b) { return (a != 0) || (b != 0); }
static double _not(double a) { return a == 0; }

static te_variable default_vars[] = {
    {"pi", (void *)0, TE_VARIABLE, 0},
    {"e", (void *)0, TE_VARIABLE, 0},
    {"sqrt", (void *)_sqrt, TE_FUNCTION1, 0},
    {"sin", (void *)_sin, TE_FUNCTION1, 0},
    {"cos", (void *)_cos, TE_FUNCTION1, 0},
    {"tan", (void *)_tan, TE_FUNCTION1, 0},
    {"asin", (void *)_asin, TE_FUNCTION1, 0},
    {"acos", (void *)_acos, TE_FUNCTION1, 0},
    {"atan", (void *)_atan, TE_FUNCTION1, 0},
    {"sinh", (void *)_sinh, TE_FUNCTION1, 0},
    {"cosh", (void *)_cosh, TE_FUNCTION1, 0},
    {"tanh", (void *)_tanh, TE_FUNCTION1, 0},
    {"exp", (void *)_exp, TE_FUNCTION1, 0},
    {"log", (void *)_log, TE_FUNCTION1, 0},
    {"log10", (void *)_log10, TE_FUNCTION1, 0},
    {"abs", (void *)_abs, TE_FUNCTION1, 0},
    {"ceil", (void *)_ceil, TE_FUNCTION1, 0},
    {"floor", (void *)_floor, TE_FUNCTION1, 0},
    {"round", (void *)_round, TE_FUNCTION1, 0},
    {"if", (void *)0, TE_PURE, 0} /* handled specially */
};

#define DEFAULT_VAR_COUNT (sizeof(default_vars) / sizeof(te_variable))

static te_expr *new_expr(int type, const void *item, void *context) {
    te_expr *ret = (te_expr *)malloc(sizeof(te_expr));
    if (!ret) return 0;
    ret->type = type;
    if (type == TE_VARIABLE) ret->item.bound = (const double *)item;
    else if (type >= TE_FUNCTION0 && type <= TE_PURE) ret->item.function = item;
    else ret->item.value = *(const double *)item;
    ret->context = context;
    return ret;
}

static void te_free_parameters(te_expr *n) {
    if (!n) return;
    if (n->type >= TE_FUNCTION0 && n->type <= TE_PURE) {
        int arity = n->type - TE_FUNCTION0;
        if (n->type == TE_PURE) arity = 3; /* if statement */
        for (int i = 0; i < arity; ++i) {
            te_free(n->param[i]);
        }
    }
}

void te_free(te_expr *n) {
    if (!n) return;
    te_free_parameters(n);
    free(n);
}

double te_eval(const te_expr *n) {
    if (!n) return 0;
    if (n->type == TE_VARIABLE) return *(n->item.bound);
    if (n->type == 0) return n->item.value;

    if (n->type == TE_PURE) {
        /* Special case for "if" */
        double cond = te_eval(n->param[0]);
        if (cond != 0) return te_eval(n->param[1]);
        else return te_eval(n->param[2]);
    }

    double v[TE_MAX_ARGS];
    int arity = n->type - TE_FUNCTION0;
    for (int i = 0; i < arity; ++i) {
        v[i] = te_eval(n->param[i]);
    }

    switch (arity) {
        case 0: return ((double (*)(void))n->item.function)();
        case 1: return ((double (*)(double))n->item.function)(v[0]);
        case 2: return ((double (*)(double, double))n->item.function)(v[0], v[1]);
        case 3: return ((double (*)(double, double, double))n->item.function)(v[0], v[1], v[2]);
        case 4: return ((double (*)(double, double, double, double))n->item.function)(v[0], v[1], v[2], v[3]);
    }
    return 0;
}

/* Parser state */
typedef struct {
    const char *start;
    const char *next;
    const te_variable *vars;
    int var_count;
    int error;
} te_parser;

static te_expr *parse_expr(te_parser *p);

static int next_token(te_parser *p, double *value) {
    while (isspace(*p->next)) p->next++;
    if (!*p->next) return 0;

    char c = *p->next;
    if (isdigit(c) || c == '.') {
        char *end;
        *value = strtod(p->next, &end);
        p->next = end;
        return 'n'; /* number */
    }

    if (isalpha(c) || c == '_') {
        const char *start = p->next;
        while (isalnum(*p->next) || *p->next == '_') p->next++;
        int len = p->next - start;
        
        /* Check for variables/functions */
        for (int i = 0; i < p->var_count; ++i) {
            if (strncmp(start, p->vars[i].name, len) == 0 && p->vars[i].name[len] == 0) {
                *value = i;
                return 'v'; /* variable/function index */
            }
        }
        
        /* Check defaults */
        for (int i = 0; i < DEFAULT_VAR_COUNT; ++i) {
             if (strncmp(start, default_vars[i].name, len) == 0 && default_vars[i].name[len] == 0) {
                *value = -1 - i;
                return 'v';
            }
        }
        
        return 'u'; /* unknown */
    }

    p->next++;
    /* Operators */
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^' || c == '(' || c == ')' || c == ',') return c;
    
    /* Logic operators */
    if (c == '=') { if (*p->next == '=') { p->next++; return 'E'; } return '='; } /* == */
    if (c == '!') { if (*p->next == '=') { p->next++; return 'N'; } return '!'; } /* != */
    if (c == '<') { if (*p->next == '=') { p->next++; return 'L'; } return '<'; } /* <= */
    if (c == '>') { if (*p->next == '=') { p->next++; return 'G'; } return '>'; } /* >= */
    if (c == '&') { if (*p->next == '&') { p->next++; return '&'; } return 0; } /* && */
    if (c == '|') { if (*p->next == '|') { p->next++; return '|'; } return 0; } /* || */

    return 0;
}

static te_expr *parse_base(te_parser *p) {
    double val;
    int type = next_token(p, &val);
    
    if (type == 'n') {
        return new_expr(0, &val, 0);
    } else if (type == 'v') {
        int idx = (int)val;
        const te_variable *v;
        if (idx >= 0) v = &p->vars[idx];
        else v = &default_vars[-1 - idx];
        
        if (v->type == TE_VARIABLE) {
            return new_expr(TE_VARIABLE, v->address, v->context);
        } else {
            /* Function call */
            if (next_token(p, &val) != '(') { p->error = 1; return 0; }
            
            te_expr *ret = new_expr(v->type, v->address, v->context);
            int arity = v->type - TE_FUNCTION0;
            if (v->type == TE_PURE) arity = 3;

            for (int i = 0; i < arity; ++i) {
                ret->param[i] = parse_expr(p);
                if (!ret->param[i]) { te_free(ret); return 0; }
                if (i < arity - 1) {
                    if (next_token(p, &val) != ',') { te_free(ret); p->error = 1; return 0; }
                }
            }
            if (next_token(p, &val) != ')') { te_free(ret); p->error = 1; return 0; }
            return ret;
        }
    } else if (type == '(') {
        te_expr *ret = parse_expr(p);
        if (next_token(p, &val) != ')') { te_free(ret); p->error = 1; return 0; }
        return ret;
    } else if (type == '-') {
        te_expr *ret = new_expr(TE_FUNCTION1, (void *)_sub, 0);
        ret->param[0] = new_expr(0, &(double){0}, 0);
        ret->param[1] = parse_base(p);
        return ret;
    } else if (type == '!') {
        te_expr *ret = new_expr(TE_FUNCTION1, (void *)_not, 0);
        ret->param[0] = parse_base(p);
        return ret;
    }
    
    p->error = 1;
    return 0;
}

static te_expr *parse_pow(te_parser *p) {
    te_expr *lhs = parse_base(p);
    if (!lhs) return 0;
    while (1) {
        const char *op = p->next;
        while (isspace(*op)) op++;
        if (*op == '^') {
            p->next = op + 1;
            te_expr *rhs = parse_base(p); /* Right associative? Actually usually pow is right associative, but let's keep simple */
            if (!rhs) { te_free(lhs); return 0; }
            te_expr *ret = new_expr(TE_FUNCTION2, (void *)_pow, 0);
            ret->param[0] = lhs;
            ret->param[1] = rhs;
            lhs = ret;
        } else break;
    }
    return lhs;
}

static te_expr *parse_mul(te_parser *p) {
    te_expr *lhs = parse_pow(p);
    if (!lhs) return 0;
    while (1) {
        const char *op = p->next;
        while (isspace(*op)) op++;
        int type = *op;
        if (type == '*' || type == '/' || type == '%') {
            p->next = op + 1;
            te_expr *rhs = parse_pow(p);
            if (!rhs) { te_free(lhs); return 0; }
            void *func = (type == '*') ? (void *)_mul : ((type == '/') ? (void *)_div : (void *)_mod);
            te_expr *ret = new_expr(TE_FUNCTION2, func, 0);
            ret->param[0] = lhs;
            ret->param[1] = rhs;
            lhs = ret;
        } else break;
    }
    return lhs;
}

static te_expr *parse_add(te_parser *p) {
    te_expr *lhs = parse_mul(p);
    if (!lhs) return 0;
    while (1) {
        const char *op = p->next;
        while (isspace(*op)) op++;
        int type = *op;
        if (type == '+' || type == '-') {
            p->next = op + 1;
            te_expr *rhs = parse_mul(p);
            if (!rhs) { te_free(lhs); return 0; }
            void *func = (type == '+') ? (void *)_add : (void *)_sub;
            te_expr *ret = new_expr(TE_FUNCTION2, func, 0);
            ret->param[0] = lhs;
            ret->param[1] = rhs;
            lhs = ret;
        } else break;
    }
    return lhs;
}

static te_expr *parse_rel(te_parser *p) {
    te_expr *lhs = parse_add(p);
    if (!lhs) return 0;
    while (1) {
        const char *op = p->next;
        while (isspace(*op)) op++;
        
        /* Check for 2-char ops first */
        int type = 0;
        if (strncmp(op, "==", 2) == 0) { type = 'E'; p->next = op + 2; }
        else if (strncmp(op, "!=", 2) == 0) { type = 'N'; p->next = op + 2; }
        else if (strncmp(op, "<=", 2) == 0) { type = 'L'; p->next = op + 2; }
        else if (strncmp(op, ">=", 2) == 0) { type = 'G'; p->next = op + 2; }
        else if (*op == '<') { type = '<'; p->next = op + 1; }
        else if (*op == '>') { type = '>'; p->next = op + 1; }
        
        if (type) {
            te_expr *rhs = parse_add(p);
            if (!rhs) { te_free(lhs); return 0; }
            void *func = 0;
            if (type == 'E') func = (void *)_equal;
            else if (type == 'N') func = (void *)_not_equal;
            else if (type == 'L') func = (void *)_less_equal;
            else if (type == 'G') func = (void *)_greater_equal;
            else if (type == '<') func = (void *)_less;
            else if (type == '>') func = (void *)_greater;
            
            te_expr *ret = new_expr(TE_FUNCTION2, func, 0);
            ret->param[0] = lhs;
            ret->param[1] = rhs;
            lhs = ret;
        } else break;
    }
    return lhs;
}

static te_expr *parse_and(te_parser *p) {
    te_expr *lhs = parse_rel(p);
    if (!lhs) return 0;
    while (1) {
        const char *op = p->next;
        while (isspace(*op)) op++;
        if (strncmp(op, "&&", 2) == 0) {
            p->next = op + 2;
            te_expr *rhs = parse_rel(p);
            if (!rhs) { te_free(lhs); return 0; }
            te_expr *ret = new_expr(TE_FUNCTION2, (void *)_and, 0);
            ret->param[0] = lhs;
            ret->param[1] = rhs;
            lhs = ret;
        } else break;
    }
    return lhs;
}

static te_expr *parse_or(te_parser *p) {
    te_expr *lhs = parse_and(p);
    if (!lhs) return 0;
    while (1) {
        const char *op = p->next;
        while (isspace(*op)) op++;
        if (strncmp(op, "||", 2) == 0) {
            p->next = op + 2;
            te_expr *rhs = parse_and(p);
            if (!rhs) { te_free(lhs); return 0; }
            te_expr *ret = new_expr(TE_FUNCTION2, (void *)_or, 0);
            ret->param[0] = lhs;
            ret->param[1] = rhs;
            lhs = ret;
        } else break;
    }
    return lhs;
}

static te_expr *parse_expr(te_parser *p) {
    return parse_or(p);
}

te_expr *te_compile(const char *expression, const te_variable *variables, int var_count, int *error) {
    te_parser p = {expression, expression, variables, var_count, 0};
    te_expr *root = parse_expr(&p);
    if (p.error || *p.next) {
        te_free(root);
        if (error) *error = (int)(p.next - p.start) + 1;
        return 0;
    }
    if (error) *error = 0;
    return root;
}

double te_interp(const char *expression, int *error) {
    te_expr *n = te_compile(expression, 0, 0, error);
    if (!n) return NAN;
    double v = te_eval(n);
    te_free(n);
    return v;
}
