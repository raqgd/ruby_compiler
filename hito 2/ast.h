#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sym_tab.h"

typedef enum {
    NODE_NUM,
    NODE_FLOAT,
    NODE_STRING,
    NODE_ID,
    NODE_ASSIGN,
    NODE_BINOP,
    NODE_UNOP,
    NODE_IF,
    NODE_WHILE,
    NODE_STMT_LIST,
    NODE_BOOL
} NodeKind;

typedef struct AST {
    NodeKind kind;

    union {
        int int_val;
        float float_val;
        char* str_val;

        struct {
            char* id;
            struct AST* expr;
        } assign;

        struct {
            int op;
            struct AST* left;
            struct AST* right;
        } binop;

        struct {
            struct AST* cond;
            struct AST* then_branch;
            struct AST* else_branch;
        } if_stmt;

        struct {
            struct AST* cond;
            struct AST* body;
        } while_stmt;

        struct {
            struct AST* first;
            struct AST* next;
        } stmt_list;

        int bool_val;
    } data;

    VarType type;

} AST;

// Funciones para construir nodos
AST* ast_create_num_node(int v);
AST* ast_create_float_node(float v);
AST* ast_create_str_node(const char* s);
AST* ast_create_id_node(const char* id);
AST* ast_create_assign_node(const char* id, AST* expr);
AST* ast_create_binop_node(int op, AST* left, AST* right);
AST* ast_create_unop_node(int op, AST* expr);
AST* ast_create_if_node(AST* cond, AST* then_branch, AST* else_branch);
AST* ast_create_while_node(AST* cond, AST* body);
AST* ast_create_stmt_list(AST* first, AST* next);
AST* ast_create_expr_stmt(AST* expr);
AST* ast_create_bool_node(int val);
AST* ast_append_stmt(AST* list, AST* stmt);

// Auxiliares
void ast_destroy(AST* node);
void print_ast(AST* node, int level);
int is_zero(AST* expr);

#endif