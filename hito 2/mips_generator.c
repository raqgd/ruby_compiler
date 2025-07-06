#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ast.h"
#include "mips_generator.h"
#include "parser.tab.h"


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

    symtable_emit_declarations(mips_output);
    
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

void mips_concat_strings() {
    fprintf(mips_output, "# Concatenar dos strings\n");
    fprintf(mips_output, "lw $a1, 0($sp)\n");        // dirección del string derecho
    fprintf(mips_output, "addu $sp, $sp, 4\n");
    fprintf(mips_output, "lw $a0, 0($sp)\n");        // dirección del string izquierdo
    fprintf(mips_output, "addu $sp, $sp, 4\n");

    fprintf(mips_output, "li $v0, 9\n");             // syscall sbrk (reserva memoria)
    fprintf(mips_output, "li $a2, 256\n");           // tamaño máximo del string resultante
    fprintf(mips_output, "syscall\n");
    fprintf(mips_output, "move $t0, $v0\n");         // dirección destino

    // Copiar primer string a $t0
    fprintf(mips_output, "move $t1, $a0\n");         // origen 1
    fprintf(mips_output, "move $t2, $t0\n");         // destino
    fprintf(mips_output, "copy_loop_1:\n");
    fprintf(mips_output, "lb $t3, 0($t1)\n");
    fprintf(mips_output, "sb $t3, 0($t2)\n");
    fprintf(mips_output, "beqz $t3, end_copy_1\n");
    fprintf(mips_output, "addi $t1, $t1, 1\n");
    fprintf(mips_output, "addi $t2, $t2, 1\n");
    fprintf(mips_output, "j copy_loop_1\n");
    fprintf(mips_output, "end_copy_1:\n");
    fprintf(mips_output, "addi $t2, $t2, -1\n"); // sobreescribimos el '\0'

    // Copiar segundo string
    fprintf(mips_output, "move $t1, $a1\n");
    fprintf(mips_output, "copy_loop_2:\n");
    fprintf(mips_output, "lb $t3, 0($t1)\n");
    fprintf(mips_output, "sb $t3, 0($t2)\n");
    fprintf(mips_output, "beqz $t3, end_copy_2\n");
    fprintf(mips_output, "addi $t1, $t1, 1\n");
    fprintf(mips_output, "addi $t2, $t2, 1\n");
    fprintf(mips_output, "j copy_loop_2\n");
    fprintf(mips_output, "end_copy_2:\n");

    // Guardar en stack la dirección del string concatenado
    fprintf(mips_output, "subu $sp, $sp, 4\n");
    fprintf(mips_output, "sw $t0, 0($sp)\n");
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

        case NODE_STMT_LIST:
            generate_code(node->data.stmt_list.first);
            generate_code(node->data.stmt_list.next);
            break;

        case NODE_BINOP:
            generate_code(node->data.binop.left);
            generate_code(node->data.binop.right);

            if (node->type == TYPE_STRING) {
                if (node->data.binop.op == PLUS) {
                    mips_concat_strings(); 
                } else {
                    fprintf(stderr, "Error: operación inválida con strings\n");
                    exit(1);
                }
            }
            else if (node->type == TYPE_FLOAT) {
                switch (node->data.binop.op) {
                    case PLUS:  mips_float_add(); break;
                    case MINUS: mips_float_sub(); break;
                    case MUL:   mips_float_mul(); break;
                    case DIV:   mips_float_div(); break;
                    default:
                        fprintf(stderr, "Operación float no soportada\n");
                        exit(1);
                }
            }
            else {
                switch (node->data.binop.op) {
                    case PLUS:  mips_add(); break;
                    case MINUS: mips_sub(); break;
                    case MUL:   mips_mul(); break;
                    case DIV:   mips_div(); break;
                    case EQ:    mips_eq();  break;
                    case NEQ:   mips_neq(); break;
                    case LT:    mips_lt();  break;
                    case GT:    mips_gt();  break;
                    case LE:    mips_le();  break;
                    case GE:    mips_ge();  break;
                    default:
                        fprintf(stderr, "Operación entera no soportada\n");
                        exit(1);
                }
            }
            break;


        case NODE_IF: {
            int label_num = label_count++;

            // Evaluar condición
            generate_code(node->data.if_stmt.cond);

            // Extraer el resultado de la condición
            fprintf(mips_output, "lw $t0, 0($sp)\n");
            fprintf(mips_output, "addu $sp, $sp, 4\n");

            if (node->data.if_stmt.else_branch) {
                // if con else
                fprintf(mips_output, "beqz $t0, else_%d\n", label_num);
                generate_code(node->data.if_stmt.else_branch);
                fprintf(mips_output, "j endif_%d\n", label_num);
                fprintf(mips_output, "else_%d:\n", label_num);
                generate_code(node->data.if_stmt.else_branch);
            } else {
                // if sin else
                fprintf(mips_output, "beqz $t0, endif_%d\n", label_num);
                generate_code(node->data.if_stmt.else_branch);
            }

            fprintf(mips_output, "endif_%d:\n", label_num);
            break;
        }

        case NODE_WHILE: {
            int label_num = label_count++;

            fprintf(mips_output, "while_start_%d:\n", label_num);
            generate_code(node->data.while_stmt.cond);
            fprintf(mips_output, "lw $t0, 0($sp)\n");
            fprintf(mips_output, "addu $sp, $sp, 4\n");
            fprintf(mips_output, "beqz $t0, while_end_%d\n", label_num);
            generate_code(node->data.while_stmt.body);
            fprintf(mips_output, "j while_start_%d\n", label_num);
            fprintf(mips_output, "while_end_%d:\n", label_num);
            break;
        }

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

void mips_float_add() {
    fprintf(mips_output, "l.s $f2, 0($sp)\n");
    fprintf(mips_output, "addiu $sp, $sp, 4\n");
    fprintf(mips_output, "l.s $f1, 0($sp)\n");
    fprintf(mips_output, "addiu $sp, $sp, 4\n");
    fprintf(mips_output, "add.s $f0, $f1, $f2\n");
    fprintf(mips_output, "addiu $sp, $sp, -4\n");
    fprintf(mips_output, "s.s $f0, 0($sp)\n");
}

void mips_float_sub() {
    fprintf(mips_output, "l.s $f2, 0($sp)\n");
    fprintf(mips_output, "addiu $sp, $sp, 4\n");
    fprintf(mips_output, "l.s $f1, 0($sp)\n");
    fprintf(mips_output, "addiu $sp, $sp, 4\n");
    fprintf(mips_output, "sub.s $f0, $f1, $f2\n");
    fprintf(mips_output, "addiu $sp, $sp, -4\n");
    fprintf(mips_output, "s.s $f0, 0($sp)\n");
}

void mips_float_mul() {
    fprintf(mips_output, "l.s $f2, 0($sp)\n");
    fprintf(mips_output, "addiu $sp, $sp, 4\n");
    fprintf(mips_output, "l.s $f1, 0($sp)\n");
    fprintf(mips_output, "addiu $sp, $sp, 4\n");
    fprintf(mips_output, "mul.s $f0, $f1, $f2\n");
    fprintf(mips_output, "addiu $sp, $sp, -4\n");
    fprintf(mips_output, "s.s $f0, 0($sp)\n");
}

void mips_float_div() {
    fprintf(mips_output, "l.s $f2, 0($sp)\n");
    fprintf(mips_output, "addiu $sp, $sp, 4\n");
    fprintf(mips_output, "l.s $f1, 0($sp)\n");
    fprintf(mips_output, "addiu $sp, $sp, 4\n");
    fprintf(mips_output, "div.s $f0, $f1, $f2\n");
    fprintf(mips_output, "addiu $sp, $sp, -4\n");
    fprintf(mips_output, "s.s $f0, 0($sp)\n");
}




