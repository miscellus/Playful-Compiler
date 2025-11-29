#ifndef VAR_TABLE_H
#define VAR_TABLE_H

#include "tokenizer.h"

#define VARKEY_MAX_LEN 256
#define INITIAL_CAPACITY 64 // Must be power of two

typedef struct VarEntry_t {
    char *key; // owned, null-terminated identifier
    double val;
} VarEntry;

typedef struct VarTable_t {
    VarEntry *items;
    size_t count;
    size_t capacity;
} VarTable;

VarEntry *vartable_find_ident(VarTable *t, Ident ident);
VarEntry *vartable_get_or_create_ident(VarTable *t, Ident ident);
void vartable_free(VarTable *t);

#endif // END VAR_TABLE_H