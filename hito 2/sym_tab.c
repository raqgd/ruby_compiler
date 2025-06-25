#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "sym_tab.h"
#include "mips_generator.h"
#include "parser.tab.h"

static Symbol *symbols = NULL;

void symtable_init() {
    symbols = NULL;
}

void symtable_destroy() {
    Symbol *s = symbols;
    while (s) {
        Symbol *next = s->next;
        free(s->name);
        free(s);
        s = next;
    }
    symbols = NULL;
}

int symtable_declare(const char *name, VarType type) {
    if (symtable_lookup(name) != TYPE_UNDEFINED)
        return 0;  // ya existe

    Symbol *s = malloc(sizeof(Symbol));
    if (!s) {
        fprintf(stderr, "Error al reservar memoria para símbolo\n");
        exit(EXIT_FAILURE);
    }
    s->name = strdup(name);
    s->type = type;
    s->next = symbols;
    symbols = s;
    return 1;
}

VarType symtable_lookup(const char *name) {
    for (Symbol *s = symbols; s; s = s->next) {
        if (strcmp(s->name, name) == 0)
            return s->type;
    }
    return TYPE_UNDEFINED;
}

int symbol_exists(const char *name) {
    return symtable_lookup(name) != TYPE_UNDEFINED;
}