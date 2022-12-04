/* main.c */

#include <string.h>

#include "ast.h"
#include "ring.h"
#include "tokenizer.h"

void ast_print(struct AST ast) { print_ast(stdout, &ast); }
void token_print(struct token token) { printf("%s(%" PRId64 ")", TOKENS_STR[token.type], token.value); }

DECLARE_RING(ast, struct AST)
DEFINE_RING(ast, struct AST)
DEFINE_RING_PRINT(ast, ast_print)
DEFINE_RING(token, struct token)
DEFINE_RING_PRINT(token, token_print)

#define RETURN_ERROR(code, msg) return printf(msg), code

const short PRECEDENCES[] = {
        [TOK_MUL] = 1,
        [TOK_DIV] = 1,
        [TOK_MINUS] = 0,
        [TOK_PLUS] = 0,
        [TOK_NEG] = 2,
};

typedef struct AST *(binop_builder)(struct AST *left, struct AST *right);

static binop_builder *binop_builders[] = {
        [TOK_MUL] = mul,
        [TOK_DIV] = divide,
        [TOK_MINUS] = sub,
        [TOK_PLUS] = add,
};

typedef struct AST *(unop_builder)(struct AST *node);

static unop_builder *unop_builders[] = {
        [TOK_NEG] = neg,
};

static struct AST *build_binop(struct ring_ast **ast_build, struct token operator) {
    struct AST* right = newnode(ring_ast_pop(ast_build));
    struct AST* left = newnode(ring_ast_pop(ast_build));
    return binop_builders[operator.type](left, right);
}

static struct AST *build_unop(struct ring_ast **ast_build, struct token operator) {
    return unop_builders[operator.type]
            (newnode(ring_ast_pop(ast_build)));

}

static struct AST *build_lit(struct ring_ast **ast_stack, struct token operator) {
    return lit(operator.value);
}

typedef struct AST *(builder)(struct ring_ast **ast_stack, struct token operator);

static builder *builders[] = {
        [AST_UNOP] = build_unop,
        [AST_BINOP] = build_binop,
        [AST_LIT] = build_lit,
};

static size_t lit_to_ast_map(struct token tok) {
    if (tok.type == TOK_LIT) return AST_LIT;
    if (is_binop(tok)) return AST_BINOP;
    if (tok.type == TOK_NEG) return AST_UNOP;
    return -1;
}

static struct AST *build_node(struct ring_ast **ast_stack, struct token tok) {
    size_t ast_type = lit_to_ast_map(tok);
    if (ast_type == -1) return NULL;
    return builders[ast_type](ast_stack, tok);
}

struct AST *build_ast(char *str)
{
  struct ring_token *tokens = NULL;
  if ((tokens = tokenize(str)) == NULL)
    RETURN_ERROR(NULL, "Tokenization error.\n");

  struct ring_ast* ast_stack = NULL;
  struct ring_token* ops_stack = NULL;

  ring_token_print(tokens);

  while (tokens != NULL) {
        struct token tok = ring_token_pop_top(&tokens);
        if (tok.type == TOK_LIT) {
            ring_ast_push(&ast_stack, *build_node(&ast_stack, tok));
        } else if (is_binop(tok) || tok.type == TOK_NEG) {
            while ((ops_stack != NULL) && (ast_stack != NULL) &&
                   (PRECEDENCES[ring_token_last(ops_stack).type] >= PRECEDENCES[tok.type])) {
                struct token operator = ring_token_pop(&ops_stack);
                if (is_binop(operator) || tok.type == TOK_NEG) {
                    ring_ast_push(&ast_stack, *build_node(&ast_stack, operator));
                } else break;
            }
            ring_token_push(&ops_stack, tok);
        } else if (tok.type == TOK_OPEN) {
            ring_token_push(&ops_stack, tok);
        } else if (tok.type == TOK_CLOSE) {
            while ((ops_stack != NULL) && ring_token_last(ops_stack).type != TOK_OPEN) {
                ring_ast_push(&ast_stack, *build_node(&ast_stack, ring_token_pop(&ops_stack)));
            }
            if ( ops_stack != NULL ) { ring_token_pop(&ops_stack); }
        }
    }
    while (ops_stack != NULL) {
        ring_ast_push(&ast_stack, *build_node(&ast_stack, ring_token_pop(&ops_stack)));
    }

    struct AST *result = newnode(ring_ast_pop(&ast_stack));
    ring_token_free(&tokens);
    ring_ast_free(&ast_stack);


    return result;
}


int main()
{
  /* char *str = "1 + 2 * (2 - -3) + 8"; */
  const int MAX_LEN = 1024;
  char str[MAX_LEN];
  if (fgets(str, MAX_LEN, stdin) == NULL)
    RETURN_ERROR(0, "Input is empty.");

  if (str[strlen(str) - 1] == '\n')
    str[strlen(str) - 1] = '\0';

  struct AST *ast = build_ast(str);

  if (ast == NULL)
    printf("AST build error.\n");
  else
  {
    print_ast(stdout, ast);
    printf("\n\n%s = %" PRId64 "\n", str, calc_ast(ast));
    p_print_ast(stdout, ast);
    printf(" = %" PRId64 "\n", calc_ast(ast));    
  }

  return 0;
}
