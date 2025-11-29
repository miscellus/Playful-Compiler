#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include "tokenizer.h"

#define VARKEY_MAX_LEN 255
#define INITIAL_CAPACITY 64 // Must be power of two

typedef struct VarEntry_t {
    char key[VARKEY_MAX_LEN + 1]; // null-terminated fixed buffer; empty string means unused
    double val;
} VarEntry;

typedef struct VarTable_t {
    VarEntry *items;
    size_t count;
    size_t capacity;
} VarTable;

VarEntry *vartable_find_ident(VarTable *t, Ident ident);
VarEntry *vartable_get_or_create_ident(VarTable *t, Ident ident);

/* FNV-1a 64-bit hash */
static uint64_t hash_bytes(const char *data, size_t len)
{
    uint64_t h = 14695981039346656037ULL;
    for (size_t i = 0; i < len; ++i) {
        h ^= (unsigned char)data[i];
        h *= 1099511628211ULL;
    }
    return h;
}

static void vartable_resize(VarTable *t, size_t newcap)
{
    VarEntry *old = t->items;
    size_t oldcap = t->capacity;

    assert(newcap > oldcap);

    VarEntry *narr = calloc(newcap, sizeof(*narr));
    t->items = narr;
    t->capacity = newcap;
    t->count = 0;

    for (size_t i = 0; i < oldcap; ++i) {
        if (old[i].key[0] != '\0') {
            /* re-insert */
            size_t klen = strlen(old[i].key);
            uint64_t h = hash_bytes(old[i].key, klen);
            size_t idx = (size_t)(h & (newcap - 1));
            while (narr[idx].key[0] != '\0') idx = (idx + 1) & (newcap - 1);
            memcpy(narr[idx].key, old[i].key, klen + 1);
            narr[idx].val = old[i].val;
            t->count++;
        }
    }

    free(old);
}

static void vartable_ensure_capacity(VarTable *t)
{
    if (!t->items) {
        t->capacity = INITIAL_CAPACITY;
        t->items = calloc(t->capacity, sizeof(*t->items));
        t->count = 0;
    }

    if (t->count * 2 >= t->capacity) { /* load factor > 0.5 */
        vartable_resize(t, t->capacity * 2);
    }
}

/* Normalize an Ident into a null-terminated buffer with max length.
   Returns pointer to staticbuf or heap (but this implementation uses stack buffer). */
static size_t ident_to_buf(char *out, size_t outcap, Ident ident)
{
    size_t len = ident.len;
    if (len > outcap - 1) len = outcap - 1;
    memcpy(out, ident.chars, len);
    out[len] = '\0';
    return len;
}

/* Find entry by ident, or NULL if not found. */
VarEntry *vartable_find_ident(VarTable *t, Ident ident)
{
    if (!t || !t->items) return NULL;

    char key[VARKEY_MAX_LEN + 1];
    size_t klen = ident_to_buf(key, sizeof(key), ident);

    uint64_t h = hash_bytes(key, klen);
    size_t idx = (size_t)(h & (t->capacity - 1));
    size_t start = idx;
    while (t->items[idx].key[0] != '\0') {
        if (strcmp(t->items[idx].key, key) == 0) return &t->items[idx];
        idx = (idx + 1) & (t->capacity - 1);
        if (idx == start) break;
    }
    return NULL;
}

/* Get or create entry for ident (returns pointer to entry). */
VarEntry *vartable_get_or_create_ident(VarTable *t, Ident ident)
{
    vartable_ensure_capacity(t);

    char key[VARKEY_MAX_LEN + 1];
    size_t klen = ident_to_buf(key, sizeof(key), ident);

    uint64_t h = hash_bytes(key, klen);
    size_t idx = (size_t)(h & (t->capacity - 1));
    while (t->items[idx].key[0] != '\0') {
        if (strcmp(t->items[idx].key, key) == 0) return &t->items[idx];
        idx = (idx + 1) & (t->capacity - 1);
    }

    /* insert new */
    memcpy(t->items[idx].key, key, klen + 1);
    t->items[idx].val = 0.0;
    t->count++;
    return &t->items[idx];
}

void vartable_free(VarTable *t)
{
    if (!t || !t->items) return;
    free(t->items);
    t->items = NULL;
    t->capacity = 0;
    t->count = 0;
}
