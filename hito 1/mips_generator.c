#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int temp_var_count = 0;
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

void mips_push_int(int value) {
    fprintf(mips_output, "li $t0, %d\n", value);
    fprintf(mips_output, "subu $sp, $sp, 4\n");
    fprintf(mips_output, "sw $t0, 0($sp)\n");
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

/* IF */
void mips_if_start(int label) {
    fprintf(mips_output, "lw $t0, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "beqz $t0, else_%d\n", label);
}

void mips_if_end(int label) {
    fprintf(mips_output, "else_%d:\n", label);
}

void mips_if_else_start(int label) {
    fprintf(mips_output, "lw $t0, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "beqz $t0, else_%d\n", label);
}

void mips_if_else_mid(int label) {
    fprintf(mips_output, "j endif_%d\n", label);
    fprintf(mips_output, "else_%d:\n", label);
}

void mips_if_else_end(int label) {
    fprintf(mips_output, "endif_%d:\n", label);
}

/* WHILE */
void mips_while_start(int label) {
    fprintf(mips_output, "while_%d:\n", label);
    fprintf(mips_output, "lw $t0, 0($sp)\n");
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "beqz $t0, endw_%d\n", label);
}

void mips_while_end(int label) {
    fprintf(mips_output, "j while_%d\n", label);
    fprintf(mips_output, "endw_%d:\n", label);
}