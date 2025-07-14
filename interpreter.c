
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>




/* The environment struct. Keeping the name and value of function or variable.
   This is linked lists*/
typedef struct Node{
    char* name;
    void* value;
    struct Node* next;
} Node;

typedef struct {
    char *param;
    int (*body)(int); 
} Function;

typedef struct Env {
    Node* begin;
    struct Env* parent;
} Env;

/* Struct for keeping tokens*/
typedef struct {
    char** tokens;
    int log_len;
    int alloc_len;
} TokenList;

/* Creating empty environment (linked lists)*/
Env* create_environment(Env* parent) {
    Env* e = malloc(sizeof(Env));
    e->begin = NULL;
    e->parent = parent;
    return e;
}

Env* glob_env;

/* Adds new nodes to linked list.*/
void add_elements_to_environment(Env* e, char* name, void* value) {
    Node* new_node = malloc(sizeof(Node));
    new_node->name = strdup(name);
    new_node->value = value;
    new_node->next = e->begin;
    e->begin = new_node;
}


/* Looks up values*/
void* lookup(Env* e, char* name) {
    for (Env* cur = e; cur != NULL; cur = cur->parent) {
        Node* curr = cur->begin;
        while (curr != NULL) {
            if (strcmp(curr->name, name) == 0) {
                return curr->value;
            }
            curr = curr->next;
        }
    }
    return NULL;
}


/* Initializes "TokenList" struct. creates an empty array with allocated length of 10. */
TokenList* create_list_of_tokens() {
    TokenList* t_list = malloc(sizeof(TokenList));
    t_list->tokens = malloc(10 * sizeof(char*));
    t_list->log_len = 0;
    t_list->alloc_len = 10;
    return t_list;
}

/* Renews "TokenList" struct. Adds token to array of tokens. */
void add_token(TokenList* t_list, char* token) {
    if (t_list->log_len >= t_list->alloc_len) {
        t_list->alloc_len *= 2;
        t_list->tokens = realloc(t_list->tokens, t_list->alloc_len * sizeof(char*));
    }
    t_list->tokens[t_list->log_len] = strdup(token);
    t_list->log_len++;
}

/* Tokenizes input and writes into t_list. */
void tokenize_input(TokenList* t_list, char input[]) {
    int i = 0;
    while (input[i] != '\0') {
        if (input[i] == '\n' || input[i] == ' ' || input[i] == '\r') {
            i++;
            continue;
        }
        if (input[i] == '\'') {
            char t[2] = {input[i], '\0'};
            add_token(t_list, t);
            i++;
            continue;
        }
        if (input[i] == '(' || input[i] == ')') {
            char t[2] = {input[i], '\0'};
            add_token(t_list, t);
            i++;
            continue;
        }
        if (input[i] == '"') {
            int start = i;  
            i++; 
            while (input[i] != '\0') {
                if (input[i] == '\\') { 
                    if (input[i+1] == '\0') {
                        fprintf(stderr, "Error: Unterminated escape sequence in string literal\n");
                        exit(1);
                    }
                    i += 2;
                } else if (input[i] == '"') {
                    break;  
                } else {
                    i++;
                }
            }
            if (input[i] == '\0') {
                fprintf(stderr, "Error: Unterminated string literal\n");
                exit(1);
            }
            int length = i - start + 1;
            char* token = malloc(length + 1);
            strncpy(token, input + start, length);
            token[length] = '\0';
            add_token(t_list, token);
            free(token);
            i++; 
            continue;
        }
        int start = i;
        while (input[i] != ' ' && input[i] != '(' && input[i] != ')' && input[i] != '\0') {
            i++;
        }
        char t[i - start + 1];
        strncpy(t, input + start, i - start);
        t[i - start] = '\0';
        add_token(t_list, t);
    }
}

int is_operator(char *symbol) {
    return (strcmp(symbol, "+") == 0 ||
            strcmp(symbol, "-") == 0 ||
            strcmp(symbol, "*") == 0 ||
            strcmp(symbol, "/") == 0 ||
            strcmp(symbol, "<") == 0 ||
            strcmp(symbol, ">") == 0 ||
            strcmp(symbol, "=") == 0 ||
            strcmp(symbol, "and") == 0 ||
            strcmp(symbol, "or") == 0);
}

void* eval(void* exp, Env* e);

typedef enum { SYMBOL, INTEGER, FLOAT, RATIONAL, STRING, LAMBDA, PAIR, OPERATOR, BUILT} types;

typedef struct data {
    types type;
    union {
        int integer;
        struct {
            int num;
            int den;
        } rational;
        double floating;
        char* symbol;
        char* string;
        struct {
           /* void (*body)(int);
            char* parameter;*/
            struct data* parameter;
            struct data* body;
            Env* e;
        } lambda;
        struct {
            void* first;
            void* second;
        } pairs;
        struct {
            struct data* (*fn)(struct data* args);
        } builtin;
    } value;
} data;

data* create_int(int val) {
    data* d = malloc(sizeof(data));
    d->type = INTEGER;
    d->value.integer = val;
    return d;
}

data* create_symbol(const char* val) {
    data* d = malloc(sizeof(data));
    d->type = SYMBOL;
    d->value.symbol = strdup(val);
    return d;
}


data* create_pair(data* first, data* second) {
    data* d = malloc(sizeof(data));
    d->type = PAIR;
    d->value.pairs.first = first;
    d->value.pairs.second = second;
    return d;
}

data* create_lambda(data* parameter, data* body, Env* e) {
    data* d = malloc(sizeof(data));
    d->type = LAMBDA;
    d->value.lambda.parameter = parameter;
    d->value.lambda.e = e;
    d->value.lambda.body = body;
    return d;
}

int gcd (int a, int b) {
    if (b == 0) {
        return abs(a);
    }
    return gcd(b, a % b);
}

data* create_rational(int num, int den) {
    if (den == 0) {
        printf("division by zero\n");
        exit(1);
    }
    int g = gcd(num, den);
    num /= g;
    den /= g;
    if (den < 0) {
        num = -num;
        den = -den;
    }
    data* d = malloc(sizeof(data));
    d->type = RATIONAL;
    d->value.rational.num = num;
    d->value.rational.den = den;
    return d;
}

data* create_string(const char* s) {
    data* d = malloc(sizeof(data));
    d->type = STRING;
    d->value.string = strdup(s);
    return d;
}

data* create_float(double val) {
    data* d = malloc(sizeof(data));
    d->type = FLOAT;
    d->value.floating = val;
    return d;
}


data* to_rational(data* n) {
    if (n->type == INTEGER) {
        return create_rational(n->value.integer, 1);
    } else if (n->type == RATIONAL) {
        return create_rational(n->value.rational.num, n->value.rational.den);
    } else {
        printf("error\n");
        exit(1);
    }
}


data* car(data* exp) {
    if (exp->type == PAIR && exp) {
        return (data*)exp->value.pairs.first;
    }
    return NULL;
}

data* cdr(data* exp) {
    if (exp->type == PAIR && exp) {
        return (data*) exp->value.pairs.second;
    }
    return NULL;
}

data* create_builtin(data* (*fn)(data* args)) {
    data* d = malloc(sizeof(data));
    d->type = BUILT;
    d->value.builtin.fn = fn;
    return d;
}

data* cons_builtin(data* args) {
    data* first = car(args);
    data* second = car(cdr(args));
    return create_pair(first, second);
}

data* car_builtin(data* args) {
    data* arg = car(args);
    if (arg == NULL || arg->type != PAIR) {
        printf("expected pair\n");
        return NULL;
    }
    return car(arg);
}

data* cdr_builtin(data* args) {
    data* arg = car(args);
    if (arg == NULL || arg->type != PAIR) {
        printf("expected pair\n");
        return NULL;
    } 
    return cdr(arg);
}

data* evaluate_list (data* arglist, Env* e) {
    if (arglist == NULL) {
        return NULL;
    } else if (arglist->type != PAIR) {
        return NULL;
    }
    data* first = (data*)eval(car(arglist), e);
    data* second = evaluate_list(cdr(arglist), e);
    return create_pair(first, second);
}

data* map_builtin(data* args) {
    if (args == NULL) {
        printf("Map: expected 2 arguments\n");
        return NULL;
    }
    data* first_ = car(args);
    data* second_ = car(cdr(args));
    if (first_ == NULL || second_ == NULL) {
        printf("Map: missing arguments\n");
        return NULL;
    }

    data* res = NULL;
    data* last = NULL;
    while( second_ != NULL && second_->type == PAIR) {
        data* element = car(second_);
        data* arg_c = create_pair(element, NULL);
        data* app = create_pair(first_, arg_c);
        
        Env* fe = NULL;
        if (first_->type == LAMBDA) {
            fe = first_ ->value.lambda.e;
        }

        data* res2 = (data*) eval(app, fe);
        data* cell = create_pair(res2, NULL);
        if (res == NULL) {
            res = cell;
            last = cell;
        } else {
            last->value.pairs.second = cell;
            last = cell;
        }
        second_ = cdr(second_);
    }
    return res;
}

data* append_builtin(data* args) {
    if (args == NULL) {
        printf("Append: expected 2 arguments\n");
        return NULL;
    }
    data* lst1 = car(args);
    data* lst2 = car(cdr(args));
    if (lst1 == NULL) {
        return lst2;
    }
    if (lst1->type != PAIR) {
        printf("Append: first argument is not a list\n");
        return NULL;
    }

    data* result = NULL;
    data* last = NULL;
    data* iter = lst1;
    while (iter != NULL && iter->type == PAIR) {
        data* cell = create_pair(car(iter), NULL);
        if (result == NULL) {
            result = cell;
            last = cell;
        } else {
            last->value.pairs.second = cell;
            last = cell;
        }
        iter = cdr(iter);
    }

    if (last != NULL)
        last->value.pairs.second = lst2;
    return result;
}

data* null_builtin(data* args) {
    data* arg = car(args);
    if (arg != NULL) {
        return create_int(0);
    } else {
        return create_int(1);
    }
}

data* length_builtin(data* args) {
    data* arg = car(args);
    int counter = 0;
    data* it = arg;
    while(it != NULL && it->type == PAIR) {
        counter++;
        it = cdr(it);
    }
    return create_int(counter);
}

data* apply_builtin(data* args) {
    data* first = car(args);
    data* second = car(cdr(args));
    if (first == NULL || second == NULL) {
        printf("Apply: expected 2 arguments\n");
        return NULL;
    }

    if (first->type == BUILT) {
        data* evaled = evaluate_list(second, glob_env);
        return first->value.builtin.fn(evaled);
    }

    data* da = create_pair(first, second);
    return (data*) eval(da, glob_env);
}

data* eval_builtin(data* args) {
    data* exp = car(args);
    if(exp == NULL) {
        printf("Eval: expected an expression\n");
        return NULL;
    }
    return (data*) eval(exp, glob_env);
}


int equal_data(data* a, data* b) {
    if (a == b) return 1;  
    if (a == NULL || b == NULL) return 0;
    if (a->type != b->type) {
        if ((a->type == INTEGER || a->type == RATIONAL || a->type == FLOAT) &&
            (b->type == INTEGER || b->type == RATIONAL || b->type == FLOAT)) {
            double da, db;
            if (a->type == INTEGER) {
                da = a->value.integer;
            } else if (a->type == FLOAT) {
                da = a->value.floating;
            } else { 
                da = (double)a->value.rational.num / a->value.rational.den;
            }
            if (b->type == INTEGER) {
                db = b->value.integer;
            } else if (b->type == FLOAT) {
                db = b->value.floating;
            } else {
                db = (double)b->value.rational.num / b->value.rational.den;
            }
            return da == db;
        }
        return 0;
    }
    switch(a->type) {
        case INTEGER:
            return a->value.integer == b->value.integer;
        case FLOAT:
            return a->value.floating == b->value.floating;
        case RATIONAL:
            return a->value.rational.num == b->value.rational.num &&
                   a->value.rational.den == b->value.rational.den;
        case STRING:
            return strcmp(a->value.string, b->value.string) == 0;
        case SYMBOL:
            return strcmp(a->value.symbol, b->value.symbol) == 0;
        case PAIR:
            return equal_data(car(a), car(b)) && equal_data(cdr(a), cdr(b));
        case LAMBDA:
            return a == b;
        case BUILT:
            return a == b;
        default:
            return 0;
    }
}

data* equal_builtin(data* args) {
    data* first = car(args);
    data* second = car(cdr(args));
    if (first == NULL || second == NULL) {
        //printf("equal?: expected 2 arguments\n");
        return NULL;
    }
    int eq = equal_data(first, second);
    return create_int(eq);
}


// Recursively free 
void free_data(data* d) {
    if (d == NULL) return;
    switch(d->type) {
        case INTEGER:
            free(d);
            break;
        case SYMBOL:
            free(d->value.symbol);
            free(d);
            break;
        case LAMBDA:
            free_data(d->value.lambda.parameter);
            free_data(d->value.lambda.body);
            free(d);
            break;
        case PAIR:
            free_data(d->value.pairs.first);
            free_data(d->value.pairs.second);
            free(d);
            break;
        case BUILT:
            free(d);
            break;
        default:
            free(d);
            break;
    }
}


data* clone_data(data* d) {
    if (!d) return NULL;
    data* copy = malloc(sizeof(data));
    copy->type = d->type;
    switch(d->type) {
        case INTEGER:
            copy->value.integer = d->value.integer;
            break;
        case FLOAT:
            copy->value.floating = d->value.floating;
            break;
        case RATIONAL:
            copy->value.rational.num = d->value.rational.num;
            copy->value.rational.den = d->value.rational.den;
            break;
        case STRING:
            copy->value.string = strdup(d->value.string);
            break;
        case SYMBOL:
            copy->value.symbol = strdup(d->value.symbol);
            break;
        case PAIR:
            copy->value.pairs.first = clone_data(car(d));
            copy->value.pairs.second = clone_data(cdr(d));
            break;
        case LAMBDA:
            copy->value.lambda.parameter = clone_data(d->value.lambda.parameter);
            copy->value.lambda.body = clone_data(d->value.lambda.body);
            copy->value.lambda.e = d->value.lambda.e; 
            break;
        case BUILT:
            copy->value.builtin.fn = d->value.builtin.fn;
            break;
        default:
            break;
    }
    return copy;
}



void* eval(void* exp, Env* e) {
    data* d = (data*) exp;
    if (d == NULL) {
        return NULL;
    }
    if (d->type == FLOAT || d->type == INTEGER || d->type == STRING || d->type == RATIONAL) {
        return d;
    } else if (d->type == SYMBOL) {
        void* val = lookup(e, d->value.symbol);
        if (val == NULL && is_operator(d->value.symbol))
            return d;
        return val;

    } else if (d->type == LAMBDA) {
        return d;
    } else if (d->type == PAIR) {
        data* first = car(d);
        if (first && first->type == SYMBOL && strcmp(first->value.symbol, "quote") == 0) {
            return car(cdr(d));
        }    
        if (first && first->type == SYMBOL) {
            if (strcmp(first->value.symbol, "lambda") == 0) {
                data* parameter = car(cdr(d));
                data* body = car(cdr(cdr(d)));
                return create_lambda(parameter, body, e);
            } else if (strcmp(first->value.symbol, "define") == 0) {
                data* var = car(cdr(d));
                if (var->type == SYMBOL) {
                    data* v_exp = car(cdr(cdr(d)));
                    data* v = (data*) eval(v_exp, e);
                    data* stored = clone_data(v);
                    add_elements_to_environment(e, var->value.symbol, stored);
                    return stored;
                } else if (var->type == PAIR) {               
                    data* f_name = car(var);               
                    data* parameters = cdr(var);              
                    data* body = car(cdr(cdr(d)));           
                    Env* lambda_env = create_environment(e);
                    data* cloned_parameters = clone_data(parameters);
                    data* cloned_body = clone_data(body);
                    data* lambda_ = create_lambda(cloned_parameters, cloned_body, lambda_env);

                    add_elements_to_environment(e, f_name->value.symbol, lambda_);
                    return lambda_;
                }
            } else if(strcmp(first->value.symbol, "if") == 0) {
                data* exp1 = car(cdr(d));         
                data* exp2 = car(cdr(cdr(d)));     
                data* exp3 = car(cdr(cdr(cdr(d))));  

                data* cond = (data*) eval(exp1, e);
                int is_true = 1;  
                if (cond && cond->type == INTEGER && cond->value.integer == 0) {
                    is_true = 0;
                }
                if (cond && cond->type == SYMBOL && strcmp(cond->value.symbol, "#f") == 0) {
                    is_true = 0;
                }
                return eval(is_true ? exp2 : exp3, e);
            }
        }

        data* func_exp = eval(car(d), e);

        if (func_exp && func_exp->type == LAMBDA) {
            data* arg_list = cdr(d);

            // data* arg1_exp = car(arg_list);
            // data* arg2_exp = car(cdr(arg_list));

            // int arg1 = *((int*)eval(arg1_exp, e));
            // int arg2 = *((int*)eval(arg2_exp, e));
            // int* res = malloc(sizeof(int));

            Env* new_e = create_environment(func_exp->value.lambda.e);
            data* params = func_exp->value.lambda.parameter;
            while(params && arg_list && params->type == PAIR && arg_list->type == PAIR ) {
                data* p = car(params);
                void* vall = eval(car(arg_list), e);
                add_elements_to_environment(new_e, p->value.symbol, vall);
                params = cdr(params);
                arg_list = cdr(arg_list);
            }
        
            return eval(func_exp->value.lambda.body, new_e);
        }
        if (func_exp && func_exp->type == BUILT) {
            if (strcmp(car(d)->value.symbol, "load") == 0) {
                return func_exp->value.builtin.fn(cdr(d));
            } else {
                data* evaled = evaluate_list(cdr(d), e);
                return func_exp->value.builtin.fn(evaled);
            }
        }


        if (is_operator(first->value.symbol) && first && first->type == SYMBOL) {
            data* arg_list = cdr(d);
            if (strcmp(first->value.symbol, "+") == 0) {
                data* result = create_rational(0, 1);
                for (data* it = arg_list; it != NULL && it->type == PAIR; it = cdr(it)) {
                    data* n = (data*) eval(car(it), e);
                    data* r = to_rational(n);
                    int a = result->value.rational.num;
                    int b = result->value.rational.den;
                    int c = r->value.rational.num;
                    int d_ = r->value.rational.den;
                    free_data(result);
                    result = create_rational(a*d_ + b*c, b*d_);
                    free_data(r);
                }
                return result;
            } else if (strcmp(first->value.symbol, "-") == 0) {
                if (arg_list == NULL || arg_list->type != PAIR) {
                    printf("expected at least 1 argument for '-'\n");
                    return NULL;
                }
                data* first_n = (data*) eval(car(arg_list), e);
                data* result = to_rational(first_n);
                data* it = cdr(arg_list);
                if (it == NULL) {
                    int a = result->value.rational.num;
                    int b = result->value.rational.den;
                    free_data(result);
                    return create_rational(-a, b);
                }
                for (; it != NULL && it->type == PAIR; it = cdr(it)) {
                    data* n = (data*) eval(car(it), e);
                    data* r = to_rational(n);
                    int a = result->value.rational.num;
                    int b = result->value.rational.den;
                    int c = r->value.rational.num;
                    int d_ = r->value.rational.den;
                    free_data(result);
                    result = create_rational(a*d_ - b*c, b*d_);
                    free_data(r);
                }
                return result;
            } else if (strcmp(first->value.symbol, "*") == 0) {
                data* result = create_rational(1, 1);
                for (data* it = arg_list; it != NULL && it->type == PAIR; it = cdr(it)) {
                    data* n = (data*) eval(car(it), e);
                    data* r = to_rational(n);
                    int a = result->value.rational.num;
                    int b = result->value.rational.den;
                    int c = r->value.rational.num;
                    int d_ = r->value.rational.den;
                    free_data(result);
                    result = create_rational(a*c, b*d_);
                    free_data(r);
                }
                return result;
            } else if (strcmp(first->value.symbol, "/") == 0) {
                if (arg_list == NULL || arg_list->type != PAIR) {
                    printf("expected at least 2 arguments for '/'\n");
                    return NULL;
                }
                data* first_n = (data*) eval(car(arg_list), e);
                data* result = to_rational(first_n);
                data* it = cdr(arg_list);
                for (; it != NULL && it->type == PAIR; it = cdr(it)) {
                    data* n = (data*) eval(car(it), e);
                    data* r = to_rational(n);
                    if (r->value.rational.num == 0) {
                        printf("division by zero\n");
                        free_data(r);
                        free_data(result);
                        return NULL;
                    }
                    int a = result->value.rational.num;
                    int b = result->value.rational.den;
                    int c = r->value.rational.num;
                    int d_ = r->value.rational.den;
                    free_data(result);
                    result = create_rational(a*d_, b*c);
                    free_data(r);
                }
                return result;
            } else if (strcmp(first->value.symbol, "<") == 0 ||
                    strcmp(first->value.symbol, ">") == 0 ||
                    strcmp(first->value.symbol, "=") == 0) {
                if (arg_list == NULL || cdr(arg_list) == NULL) {
                    printf("expected at least 2 arguments for relational operator\n");
                    return NULL;
                }
                int chain_result = 1;
                data* prev_node = (data*) eval(car(arg_list), e);
                double prev_val = 0;
                if (prev_node->type == INTEGER)
                    prev_val = prev_node->value.integer;
                else if (prev_node->type == RATIONAL)
                    prev_val = (double) prev_node->value.rational.num / prev_node->value.rational.den;
                for (data* it = cdr(arg_list); it != NULL && it->type == PAIR; it = cdr(it)) {
                    data* curr_node = (data*) eval(car(it), e);
                    double curr_val = 0;
                    if (curr_node->type == INTEGER)
                        curr_val = curr_node->value.integer;
                    else if (curr_node->type == RATIONAL)
                        curr_val = (double) curr_node->value.rational.num / curr_node->value.rational.den;
                    if (strcmp(first->value.symbol, "<") == 0) {
                        if (!(prev_val < curr_val)) { chain_result = 0; break; }
                    } else if (strcmp(first->value.symbol, ">") == 0) {
                        if (!(prev_val > curr_val)) { chain_result = 0; break; }
                    } else if (strcmp(first->value.symbol, "=") == 0) {
                        if (!(prev_val == curr_val)) { chain_result = 0; break; }
                    }
                    prev_val = curr_val;
                }
                return create_int(chain_result);
            } else if (strcmp(first->value.symbol, "and") == 0 ||
                    strcmp(first->value.symbol, "or") == 0) {
                if (arg_list == NULL || arg_list->type != PAIR) {
                    printf("expected at least 1 argument for logical operator\n");
                    return NULL;
                }
                int result;
                if (strcmp(first->value.symbol, "and") == 0)
                    result = 1;
                else
                    result = 0;
                for (data* it = arg_list; it != NULL && it->type == PAIR; it = cdr(it)) {
                    data* n = (data*) eval(car(it), e);
                    int v = n->value.integer;
                    if (strcmp(first->value.symbol, "and") == 0) {
                        result = result && v;
                    } else {
                        result = result || v;
                    }
                }
                return create_int(result);
            }
            return NULL;
        }

        return NULL;
    } else {
        return NULL;
    }
}

data* parse_func(TokenList* tokens, int* ind);


data* parse_func(TokenList* tokens, int* ind) {
    if (*ind >= tokens->log_len) {
        return NULL;
    }
    char* tk = tokens->tokens[*ind];
    (*ind)++;
    if (strcmp(tk, "'") == 0) {
        data* quoted_expr = parse_func(tokens, ind);
        data* quote_sym = create_symbol("quote");
        return create_pair(quote_sym, create_pair(quoted_expr, NULL));
    }
    if (strcmp(tk, "(") == 0) {
        data* head = NULL;
        data* tail = NULL;
        while (*ind < tokens->log_len && strcmp(tokens->tokens[*ind], ")") != 0) {
            data* elem = parse_func(tokens, ind);
            if (elem == NULL)
                return NULL;
            if (head == NULL) {
                head = create_pair(elem, NULL);
                tail = head;
            } else {
                tail->value.pairs.second = create_pair(elem, NULL);
                tail = tail->value.pairs.second;
            }
        }
        if (*ind >= tokens->log_len) {
            printf("Error: missing closing parenthesis\n");
            return NULL;
        }
        (*ind)++; 
        return head;
    }
    if (strcmp(tk, ")") == 0) {
        printf("Error: unexpected ')'\n");
        return NULL;
    }
    
    if (tk[0] == '"' && tk[strlen(tk) - 1] == '"') {
         int len = strlen(tk);
         char* inner = malloc(len - 1); 
         strncpy(inner, tk + 1, len - 2);
         inner[len - 2] = '\0';
         data* d = create_string(inner);
         free(inner);
         return d;
    }
    char* end;
    long num = strtol(tk, &end, 10);
    if (*end == '\0') {
         return create_int((int)num);
    }
    int is_float = 0;
    for (char* p = tk; *p != '\0'; p++) {
         if (*p == '.') {
              is_float = 1;
              break;
         }
    }
    if (is_float) {
         double d = atof(tk);
         return create_float(d);
    }
    return create_symbol(tk);
}

data* parse(TokenList* tokens) {
    int ind = 0;
    return parse_func(tokens, &ind);
}

void print_data(data* d) {
    if (!d) {
        printf("NULL");
        return;
    }
    switch(d->type) {
        case INTEGER:
            printf("%d", d->value.integer);
            break;
        case RATIONAL:
            if (d->value.rational.den == 1)
                printf("%d", d->value.rational.num);
            else
                printf("%d/%d", d->value.rational.num, d->value.rational.den);
            break;
        case FLOAT:
            printf("%g", d->value.floating);
            break;
        case STRING:
            printf("\"%s\"", d->value.string);
            break;
        case SYMBOL:
            printf("%s", d->value.symbol);
            break;
        case LAMBDA:
            printf("<lambda>");
            break;
        case BUILT:
            printf("<builtin>");
            break;
        case PAIR: {
            printf("(");
            data* iter = d;
            int first = 1;
            while (iter && iter->type == PAIR) {
                if (!first) printf(" ");
                print_data(car(iter));
                first = 0;
                data* rest = cdr(iter);
                if (!rest)
                    iter = NULL;
                else if (rest->type == PAIR)
                    iter = rest;
                else {
                    printf(" . ");
                    print_data(rest);
                    iter = NULL;
                }
            }
            printf(")");
            break;
        }
        default:
            printf("unknown");
    }
}


void free_token_list(TokenList* t_list) {
    if (t_list) {
        for (int i = 0; i < t_list->log_len; i++) {
            free(t_list->tokens[i]);
        }
        free(t_list->tokens);
        free(t_list);
    }
}

void free_environment(Env* env) {
    if (env == NULL) return;
    Node* curr = env->begin;
    while (curr != NULL) {
        Node* next = curr->next;
        free(curr->name); 
        free(curr);       
        curr = next;
    }
    free_environment(env->parent);
    free(env);  
}



data* load_builtin(data* args) {
    data* fileNameNode = car(args);
    data* evaluated = (data*) eval(fileNameNode, glob_env);
    if (evaluated == NULL ||
       (evaluated->type != SYMBOL && evaluated->type != STRING)) {
        fprintf(stderr, "load: expected a file name as a symbol or string\n");
        return NULL;
    }
    
    char* fileText;
    if (evaluated->type == STRING)
        fileText = evaluated->value.string;
    else
        fileText = evaluated->value.symbol;
    
    char* filename = strdup(fileText);
    
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "load: cannot open file %s\n", filename);
        free(filename);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    rewind(f);
    
    char* buffer = malloc(fileSize + 1);
    if (!buffer) {
        fprintf(stderr, "load: memory allocation error\n");
        fclose(f);
        free(filename);
        return NULL;
    }
    size_t bytesRead = fread(buffer, 1, fileSize, f);
    buffer[bytesRead] = '\0';
    fclose(f);
    free(filename);
    
    TokenList* t_list = create_list_of_tokens();
    tokenize_input(t_list, buffer);
    free(buffer);
    
    int pos = 0;
    while (pos < t_list->log_len) {
        data* ast = parse_func(t_list, &pos);
        if (ast == NULL)
            break;
        data* res = (data*) eval(ast, glob_env);
        if (!(ast->type == PAIR &&
              car(ast)->type == SYMBOL &&
              strcmp(car(ast)->value.symbol, "define") == 0)) {
            if (res && !(res->type == SYMBOL && strcmp(res->value.symbol, "#<unspecified>") == 0)) {
                print_data(res);
                printf("\n");
            }
        }
        free_data(ast);
    }
    
    free_token_list(t_list);
    
    return create_symbol("#<unspecified>");
}





int main() {
    Env* env = create_environment(NULL);
    glob_env = env;
    
    add_elements_to_environment(env, "cons", create_builtin(cons_builtin));
    add_elements_to_environment(env, "car", create_builtin(car_builtin));
    add_elements_to_environment(env, "cdr", create_builtin(cdr_builtin));
    add_elements_to_environment(env, "map", create_builtin(map_builtin));
    add_elements_to_environment(env, "append", create_builtin(append_builtin));
    add_elements_to_environment(env, "null?", create_builtin(null_builtin));
    add_elements_to_environment(env, "length", create_builtin(length_builtin));
    add_elements_to_environment(env, "apply", create_builtin(apply_builtin));
    add_elements_to_environment(env, "eval", create_builtin(eval_builtin));
    add_elements_to_environment(env, "load", create_builtin(load_builtin));
    add_elements_to_environment(env, "equal?", create_builtin(equal_builtin));

    
    printf("Scheme Interpreter. '(exit)' to quit.\n");
    
    char* line = NULL;
    size_t linecap = 0;
    
    while (1) {
        printf("> ");
        ssize_t nread = getline(&line, &linecap, stdin);
        if (nread == -1)
            break;
        if (line[nread-1] == '\n')
            line[nread-1] = '\0';
        if (strcmp(line, "(exit)") == 0)
            break;
        
        TokenList* t_list = create_list_of_tokens();
        tokenize_input(t_list, line);
        data* ast = parse(t_list);
        if (ast == NULL) {
            printf("Parse error.\n");
            continue;
        }
    data* result = (data*) eval(ast, env);
if (ast->type == PAIR && car(ast)->type == SYMBOL &&
    strcmp(car(ast)->value.symbol, "define") == 0) {
} else if (result != NULL &&
           !(result->type == SYMBOL &&
             strcmp(result->value.symbol, "#<unspecified>") == 0)) {
    print_data(result);
    printf("\n");
}
free_data(ast);

        free_token_list(t_list);
    }
    
    free(line);
    free_environment(env);
    return 0;
}



