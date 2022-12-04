/* ast.c */

#include <stdlib.h>

#include "ast.h"

struct AST *newnode(struct AST ast) {
  struct AST *const node = malloc(sizeof(struct AST));
  *node = ast;
  return node;
}

struct AST _lit(int64_t value) {
  return (struct AST){AST_LIT, .as_literal = {value}};
}

struct AST *lit(int64_t value) {
  return newnode( _lit( value ) );
}
struct AST _unop(enum unop_type type, struct AST *operand) {
  return (struct AST){AST_UNOP, .as_unop = {type, operand}};
}

struct AST *unop(enum unop_type type, struct AST *operand) {
  return newnode( _unop( type, operand ) );
}

struct AST _binop(enum binop_type type, struct AST *left, struct AST *right) {
  return (struct AST){AST_BINOP, .as_binop = {type, left, right}};
}
struct AST *binop(enum binop_type type, struct AST *left, struct AST *right) {
  return newnode( _binop( type, left, right ) );
}

static const char *BINOPS[] = {
    [BIN_PLUS] = "+", [BIN_MINUS] = "-", [BIN_MUL] = "*", [BIN_DIV] = "/" };
static const char *UNOPS[] = {[UN_NEG] = "-"};

typedef int64_t (binop_calc)( struct AST* left, struct AST* right );
typedef int64_t (unop_calc)( struct AST* param );

int64_t calc_plus( struct AST* left, struct AST* right );
int64_t calc_minus( struct AST* left, struct AST* right );
int64_t calc_mul( struct AST* left, struct AST* right );
int64_t calc_div( struct AST* left, struct AST* right );
int64_t calc_neg( struct AST* param );


static binop_calc* BINOPS_CALC[] = {
    [BIN_PLUS] = calc_plus, [BIN_MINUS] = calc_minus, [BIN_MUL] = calc_mul, [BIN_DIV] = calc_div };
static unop_calc* UNOPS_CALC[] = {[UN_NEG] = calc_neg};

int64_t calc_binop( struct AST * ast ){
  return BINOPS_CALC[ast->as_binop.type]( ast->as_binop.left, ast->as_binop.right );
}

int64_t calc_unop( struct AST * ast ){
  return UNOPS_CALC[ast->as_unop.type](ast->as_unop.operand);
}

int64_t calc_lit( struct AST * ast ){
  return ast->as_literal.value;
}

typedef int64_t (calcs)( struct AST * ast );

static calcs *calcs_ast[] = {
    [AST_BINOP] = calc_binop, [AST_UNOP] = calc_unop, [AST_LIT] = calc_lit };


int64_t calc_ast(struct AST *ast) {
  if (ast)
    return calcs_ast[ast->type](ast);
  else{
    fprintf( stderr, "Occured an error when ast has computed." );
    return 1;
  }
}

int64_t calc_plus( struct AST* left, struct AST* right ){
  return calc_ast( left ) + calc_ast( right );
}

int64_t calc_minus( struct AST* left, struct AST* right ){
  return calc_ast( left ) - calc_ast( right );
}

int64_t calc_mul( struct AST* left, struct AST* right ){
  return calc_ast( left ) * calc_ast( right );
}

int64_t calc_div( struct AST* left, struct AST* right ){
  return calc_ast( left ) / calc_ast( right );
}

int64_t calc_neg( struct AST* param ){
  return - (calc_ast( param ));
}

typedef void (printer) (FILE *, struct AST *);


static void print_binop(FILE *f, struct AST *ast) {
  fprintf(f, "(");
  print_ast(f, ast->as_binop.left);
  fprintf(f, ")");
  fprintf(f, "%s", BINOPS[ast->as_binop.type]);
  fprintf(f, "(");
  print_ast(f, ast->as_binop.right);
  fprintf(f, ")");
}
static void print_unop(FILE *f, struct AST *ast) {
  fprintf(f, "%s(", UNOPS[ast->as_unop.type]);
  print_ast(f, ast->as_unop.operand);
  fprintf(f, ")");
}
static void print_lit(FILE *f, struct AST *ast) {
  fprintf(f, "%" PRId64, ast->as_literal.value);
}

static printer *ast_printers[] = {
    [AST_BINOP] = print_binop, [AST_UNOP] = print_unop, [AST_LIT] = print_lit };

static void p_print_binop(FILE *f, struct AST *ast) {
  p_print_ast(f, ast->as_binop.left);
  p_print_ast(f, ast->as_binop.right);
  fprintf(f, "%s", BINOPS[ast->as_binop.type]);
}
static void p_print_unop(FILE *f, struct AST *ast) {
  p_print_ast(f, ast->as_unop.operand);
  fprintf(f, "%s", UNOPS[ast->as_unop.type]);
}

static printer *p_ast_printers[] = {
    [AST_BINOP] = p_print_binop, [AST_UNOP] = p_print_unop, [AST_LIT] = print_lit };

void print_ast(FILE *f, struct AST *ast) {
  if (ast)
    ast_printers[ast->type](f, ast);
  else
    fprintf(f, "<NULL>");
}

void p_print_ast(FILE *f, struct AST *ast) {
  if (ast)
    p_ast_printers[ast->type](f, ast);
  else
    fprintf(f, "<NULL>");
}
