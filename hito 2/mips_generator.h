#ifndef MIPS_GENERATOR_H
#define MIPS_GENERATOR_H

#include "ast.h" // Para usar AST* si lo necesitas en funciones futuras

// Inicialización y finalización del código MIPS
void init_mips();
void end_mips();

// Declaración de variables (en .data)
void declare_variable(const char* id, const char* type);  // "int" o "float"

// Carga de valores inmediatos en la pila
void mips_push_int(int value);
void mips_push_float(float value);
void mips_push_string(const char* str);  // Añadido para compatibilidad con cadenas

// Operaciones aritméticas
void mips_add();
void mips_sub();
void mips_mul();
void mips_div();

// Operaciones aritméticas float
void mips_float_add();
void mips_float_sub();
void mips_float_mul();
void mips_float_div();

// Comparaciones (enteros)
void mips_eq();
void mips_neq();
void mips_lt();
void mips_gt();
void mips_le();
void mips_ge();

// Asignaciones
void mips_assign(const char *id);
void mips_assign_float(const char *id);

// Uso de variables (carga en registros)
void mips_use_var(const char *id);
void mips_use_var_float(const char *id);

// Resultado de expresiones
void mips_expr_result(int value);
void mips_expr_result_float();

// Control de flujo condicional
void mips_if_start(int label);
void mips_if_end(int label);

void mips_if_else_start(int label);
void mips_if_else_mid(int label);
void mips_if_else_end(int label);

// Bucle while
void mips_while_start(int label);
void mips_while_end(int label);

// Generación principal desde AST
void generate_mips_from_ast(AST* root);

#endif