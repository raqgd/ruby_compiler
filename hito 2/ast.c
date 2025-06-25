#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "sym_tab.h"
#include "mips_generator.h"
#include "parser.tab.h"

AST* ast_create_num_node(int v) {
    AST* n = malloc(sizeof(AST));
    n->kind = NODE_NUM;
    n->type = TYPE_INT;
    n->data.int_val = v;
    return n;
}

AST* ast_create_float_node(float v) {
    AST* n = malloc(sizeof(AST));
    n->kind = NODE_FLOAT;
    n->type = TYPE_FLOAT;
    n->data.float_val = v;
    return n;
}

AST* ast_create_string_node(const char* s) {
    AST* n = malloc(sizeof(AST));
    n->kind = NODE_STRING;
    n->type = TYPE_STRING;
    n->data.str_val = strdup(s);
    return n;
}

AST* ast_create_id_node(const char* id) {
    AST* n = malloc(sizeof(AST));
    n->kind = NODE_ID;
    n->type = symtable_lookup(id);
    n->data.str_val = strdup(id);
    return n;
}

AST* ast_create_assign_node(const char* id, AST* expr) {
    AST* n = malloc(sizeof(AST));
    n->kind = NODE_ASSIGN;
    n->data.assign.id = strdup(id);
    n->data.assign.expr = expr;
    n->type = expr->type;
    return n;
}

AST* ast_create_binop_node(int op, AST* left, AST* right) {
    AST* n = malloc(sizeof(AST));
    n->kind = NODE_BINOP;
    n->data.binop.op = op;
    n->data.binop.left = left;
    n->data.binop.right = right;

    if (left->type == TYPE_STRING || right->type == TYPE_STRING) {
        if (op == PLUS)
            n->type = TYPE_STRING;
        else {
            fprintf(stderr, "Error: operación inválida con string\n");
            exit(1);
        }
    } else if (left->type == TYPE_FLOAT || right->type == TYPE_FLOAT) {
        n->type = TYPE_FLOAT;
    } else {
        n->type = TYPE_INT;
    }
    return n;
}

AST* ast_create_if_node(AST* cond, AST* then_b, AST* else_b) {
    AST* n = malloc(sizeof(AST));
    n->kind = NODE_IF;
    n->data.if_stmt.cond = cond;
    n->data.if_stmt.then_branch = then_b;
    n->data.if_stmt.else_branch = else_b;
    return n;
}

AST* ast_create_while_node(AST* cond, AST* body) {
    AST* n = malloc(sizeof(AST));
    n->kind = NODE_WHILE;
    n->data.while_stmt.cond = cond;
    n->data.while_stmt.body = body;
    return n;
}

AST* ast_create_bool_node(int b) {
    AST* n = malloc(sizeof(AST));
    n->kind = NODE_BOOL;
    n->type = TYPE_BOOL;
    n->data.bool_val = b;
    return n;
}

AST* ast_create_expr_stmt(AST* expr) {
    AST* n = malloc(sizeof(AST));
    n->kind = NODE_STMT_LIST;
    n->data.stmt_list.first = expr;
    n->data.stmt_list.next = NULL;
    n->type = expr->type;
    return n;
}

AST* ast_append_stmt(AST* list, AST* stmt) {
    if (!list) return stmt;

    AST* cur = list;
    while (cur->kind == NODE_STMT_LIST && cur->data.stmt_list.next) {
        cur = cur->data.stmt_list.next;
    }
    if (cur->kind == NODE_STMT_LIST) {
        cur->data.stmt_list.next = stmt;
    } else {
        AST* newlist = malloc(sizeof(AST));
        newlist->kind = NODE_STMT_LIST;
        newlist->data.stmt_list.first = list;
        newlist->data.stmt_list.next = stmt;
        return newlist;
    }
    return list;
}

int is_zero(AST* node) {
    if (!node) return 0;

    if (node->kind == NODE_NUM)
        return node->data.int_val == 0;
    if (node->kind == NODE_FLOAT)
        return node->data.float_val == 0.0f;

    return 0;
}

void ast_destroy(AST* node) {
    if (!node) return;

    switch (node->kind) {
        case NODE_NUM:
        case NODE_FLOAT:
        case NODE_BOOL:
            break;

        case NODE_STRING:
        case NODE_ID:
            free(node->data.str_val);
            break;

        case NODE_ASSIGN:
            free(node->data.assign.id);
            ast_destroy(node->data.assign.expr);
            break;

        case NODE_BINOP:
        case NODE_UNOP:
            ast_destroy(node->data.binop.left);
            ast_destroy(node->data.binop.right);
            break;

        case NODE_IF:
            ast_destroy(node->data.if_stmt.cond);
            ast_destroy(node->data.if_stmt.then_branch);
            ast_destroy(node->data.if_stmt.else_branch);
            break;

        case NODE_WHILE:
            ast_destroy(node->data.while_stmt.cond);
            ast_destroy(node->data.while_stmt.body);
            break;

        case NODE_STMT_LIST:
            ast_destroy(node->data.stmt_list.first);
            ast_destroy(node->data.stmt_list.next);
            break;
    }

    free(node);
}