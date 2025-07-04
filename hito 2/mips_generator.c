#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ast.h"
#include "mips_generator.h"

static int temp_var_count = 0;
static int float_const_count = 0;
static FILE* mips_output;

void init_mips() {
    mips_output = fopen("output.asm", "w");
    if (!mips_output) {
        fprintf(stderr, "Error al abrir output.asm\n");
        exit(1);
    }
    fprintf(mips_output, ".data\n");
    fprintf(mips_output, "newline: .asciiz \"\\n\"\n");
    fprintf(mips_output, ".text\n");
    fprintf(mips_output, ".globl main\n");
    fprintf(mips_output, "main:\n");
}

void end_mips() {
    fprintf(mips_output, "li $v0, 10\n");
    fprintf(mips_output, "syscall\n");
    fclose(mips_output);
}

void declare_variable(const char* id, const char* type) {
    if (strcmp(type, "int") == 0) {
        fprintf(mips_output, "%s: .word 0\n", id);
    } else if (strcmp(type, "float") == 0) {
        fprintf(mips_output, "%s: .float 0.0\n", id);
    }
}

void mips_push_int(int value) {
    fprintf(mips_output, "li $t0, %d\n", value);
    fprintf(mips_output, "subu $sp, $sp, 4\n");
    fprintf(mips_output, "sw $t0, 0($sp)\n");
}

void mips_push_string(const char* str) {
    fprintf(mips_output, ".data\nstring_label_%d: .asciiz \"%s\"\n.text\n", temp_var_count, str);
    fprintf(mips_output, "la $a0, string_label_%d\n", temp_var_count);
    fprintf(mips_output, "li $v0, 4\n");
    fprintf(mips_output, "syscall\n");
    temp_var_count++;
}

void mips_binary_op(const char *op_instr) {
    fprintf(mips_output, "lw $t1, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "lw $t0, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "%s $t2, $t0, $t1\n", op_instr);
    fprintf(mips_output, "subu $sp, $sp, 4\n");
    fprintf(mips_output, "sw $t2, 0($sp)\n");
}

void mips_add() { mips_binary_op("add"); }
void mips_sub() { mips_binary_op("sub"); }
void mips_mul() { mips_binary_op("mul"); }
void mips_div() { mips_binary_op("div"); }

void mips_rel_op(const char *op_instr) {
    fprintf(mips_output, "lw $t1, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "lw $t0, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "%s $t2, $t0, $t1\n", op_instr);
    fprintf(mips_output, "subu $sp, $sp, 4\n");
    fprintf(mips_output, "sw $t2, 0($sp)\n");
}

void mips_eq() {
    fprintf(mips_output, "lw $t1, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "lw $t0, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "li $t2, 0\n");
    fprintf(mips_output, "beq $t0, $t1, true_eq_%d\n", temp_var_count);
    fprintf(mips_output, "j end_eq_%d\n", temp_var_count);
    fprintf(mips_output, "true_eq_%d:\n", temp_var_count);
    fprintf(mips_output, "li $t2, 1\n");
    fprintf(mips_output, "end_eq_%d:\n", temp_var_count);
    fprintf(mips_output, "subu $sp, $sp, 4\n");
    fprintf(mips_output, "sw $t2, 0($sp)\n");
    temp_var_count++;
}

void mips_neq() {
    fprintf(mips_output, "lw $t1, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "lw $t0, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "li $t2, 0\n");
    fprintf(mips_output, "bne $t0, $t1, true_neq_%d\n", temp_var_count);
    fprintf(mips_output, "j end_neq_%d\n", temp_var_count);
    fprintf(mips_output, "true_neq_%d:\n", temp_var_count);
    fprintf(mips_output, "li $t2, 1\n");
    fprintf(mips_output, "end_neq_%d:\n", temp_var_count);
    fprintf(mips_output, "subu $sp, $sp, 4\n");
    fprintf(mips_output, "sw $t2, 0($sp)\n");
    temp_var_count++;
}

void mips_lt() { mips_rel_op("slt"); }

void mips_gt() {
    fprintf(mips_output, "lw $t1, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "lw $t0, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "slt $t2, $t1, $t0\n");
    fprintf(mips_output, "subu $sp, $sp, 4\n");
    fprintf(mips_output, "sw $t2, 0($sp)\n");
}

void mips_le() {
    mips_gt();
    fprintf(mips_output, "xori $t2, $t2, 1\n");
    fprintf(mips_output, "sw $t2, 0($sp)\n");
}

void mips_ge() {
    mips_lt();
    fprintf(mips_output, "xori $t2, $t2, 1\n");
    fprintf(mips_output, "sw $t2, 0($sp)\n");
}

void mips_assign(const char *id) {
    fprintf(mips_output, "lw $t0, 0($sp)\n");
    fprintf(mips_output, "la $t1, %s\n", id);
    fprintf(mips_output, "sw $t0, 0($t1)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
}

void mips_use_var(const char *id) {
    fprintf(mips_output, "la $t0, %s\n", id);
    fprintf(mips_output, "lw $t1, 0($t0)\n");
    fprintf(mips_output, "subu $sp, $sp, 4\n");
    fprintf(mips_output, "sw $t1, 0($sp)\n");
}

void mips_expr_result(int value) {
    fprintf(mips_output, "li $v0, 1\n");
    fprintf(mips_output, "lw $a0, 0($sp)\n");
    fprintf(mips_output, "syscall\n");
    fprintf(mips_output, "li $v0, 4\n");
    fprintf(mips_output, "la $a0, newline\n");
    fprintf(mips_output, "syscall\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
}

static int label_count = 0;

static void generate_code(AST* node) {
    if (!node) return;

    switch (node->kind) {
        case NODE_NUM:
            mips_push_int(node->data.int_val);
            break;

        case NODE_FLOAT:
            mips_push_float(node->data.float_val);  // esta función la crearemos más adelante
            break;

        case NODE_BOOL:
            mips_push_int(node->data.bool_val);
            break;

        case NODE_STRING:
            mips_push_string(node->data.str_val);
            break;

        case NODE_ID:
            if (node->type == TYPE_FLOAT)
                mips_use_var_float(node->data.str_val);  // esta función la crearemos más adelante
            else
                mips_use_var(node->data.str_val);
            break;

        case NODE_ASSIGN:
            generate_code(node->data.assign.expr);
            if (node->type == TYPE_FLOAT)
                mips_assign_float(node->data.assign.id);  // esta función la crearemos más adelante
            else
                mips_assign(node->data.assign.id);
            break;

        default:
            fprintf(stderr, "Error: nodo no implementado aún en generate_code\n");
            break;
    }
}

void generate_mips_from_ast(AST* root) {
    generate_code(root);
}

void mips_push_float(float value) {
    fprintf(mips_output, ".data\nfloat_const_%d: .float %f\n.text\n", float_const_count, value);
    fprintf(mips_output, "l.s $f0, float_const_%d\n", float_const_count);
    fprintf(mips_output, "subu $sp, $sp, 4\n"); // Reservar espacio
    fprintf(mips_output, "s.s $f0, 0($sp)\n");  // Guardar valor
    float_const_count++;
}

void mips_assign_float(const char* id) {
    fprintf(mips_output, "l.s $f0, 0($sp)\n");
    fprintf(mips_output, "la $t0, %s\n", id);
    fprintf(mips_output, "s.s $f0, 0($t0)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
}

void mips_use_var_float(const char* id) {
    fprintf(mips_output, "la $t0, %s\n", id);
    fprintf(mips_output, "l.s $f0, 0($t0)\n");
    fprintf(mips_output, "subu $sp, $sp, 4\n");
    fprintf(mips_output, "s.s $f0, 0($sp)\n");
}