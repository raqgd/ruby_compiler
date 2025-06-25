#ifndef SYM_TAB_H
#define SYM_TAB_H

typedef enum {
    TYPE_UNDEFINED = -1,
    TYPE_ERROR,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL
} VarType;

typedef struct Symbol {
    char *name;
    VarType type;
    struct Symbol *next;
} Symbol;

// Inicialización y destrucción
void symtable_init();
void symtable_destroy();

// Declaración y consulta
int symtable_declare(const char *name, VarType type);
VarType symtable_lookup(const char *name);
int symbol_exists(const char *name);

#endif