#ifndef MIPS_GENERATOR_H
#define MIPS_GENERATOR_H

void init_mips();
void end_mips();

void mips_push_int(int value);
void mips_add();
void mips_sub();
void mips_mul();
void mips_div();

void mips_eq();
void mips_neq();
void mips_lt();
void mips_gt();
void mips_le();
void mips_ge();

void mips_assign(const char *id);
void mips_use_var(const char *id);
void mips_expr_result(int value);

void mips_if_start(int label);
void mips_if_end(int label);
void mips_if_else_start(int label);
void mips_if_else_mid(int label);
void mips_if_else_end(int label);

void mips_while_start(int label);
void mips_while_end(int label);

#endif