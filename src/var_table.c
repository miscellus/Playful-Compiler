#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

typedef struct VarEntry_t {
    char *key; // owned, null-terminated identifier
    double val;
} VarEntry;

typedef struct VarTable_t {
    VarEntry *items;
    size_t count;
    size_t capacity;
} VarTable;

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

    VarEntry *narr = calloc(newcap, sizeof(*narr));
    t->items = narr;
    t->capacity = newcap;
    t->count = 0;

    for (size_t i = 0; i < oldcap; ++i) {
        if (old[i].key) {
            char *key = old[i].key;
            double val = old[i].val;
            uint64_t h = hash_bytes(key, strlen(key));
            size_t idx = (size_t)(h & (newcap - 1));
            while (narr[idx].key) idx = (idx + 1) & (newcap - 1);
            narr[idx].key = key; /* transfer ownership */
            narr[idx].val = val;
            t->count++;
        }
    }

    free(old);
}


static void vartable_init_if_needed(VarTable *t)
{
    if (!t->items) {
        size_t cap = 16;
        t->items = calloc(cap, sizeof(*t->items));
        t->capacity = cap;
        t->count = 0;
    }
}

static void vartable_ensure_capacity(VarTable *t)
{
    vartable_init_if_needed(t);
    if (t->count * 2 >= t->capacity) {
        vartable_resize(t, t->capacity * 2);
    }
}

/* Find entry by key (does not create). key may be non-null-terminated; pass len.
   If len == 0 the function treats key as null-terminated. */
static VarEntry *vartable_find(VarTable *t, const char *key, size_t len)
{
    if (t->items == NULL) return NULL;

    char tmpbuf[128];
    const char *kptr = key;
    size_t klen = len;
    if (klen == 0) klen = strlen(key);

    if (klen < sizeof(tmpbuf)) {
        memcpy(tmpbuf, key, klen);
        tmpbuf[klen] = '\0';
        kptr = tmpbuf;
    } else {
        char *heap = malloc(klen + 1);
        memcpy(heap, key, klen);
        heap[klen] = '\0';
        kptr = heap;
        uint64_t h = hash_bytes(kptr, klen);
        size_t idx = (size_t)(h & (t->capacity - 1));
        size_t start = idx;
        while (t->items[idx].key) {
            if (strcmp(t->items[idx].key, kptr) == 0) {
                free(heap);
                return &t->items[idx];
            }
            idx = (idx + 1) & (t->capacity - 1);
            if (idx == start) break;
        }
        free(heap);
        return NULL;
    }

    uint64_t h = hash_bytes(kptr, klen);
    size_t idx = (size_t)(h & (t->capacity - 1));
    size_t start = idx;
    while (t->items[idx].key) {
        if (strcmp(t->items[idx].key, kptr) == 0) return &t->items[idx];
        idx = (idx + 1) & (t->capacity - 1);
        if (idx == start) break;
    }
    return NULL;
}

/* Get or create entry for a key. key may be non-null-terminated; pass len.
   If len == 0 treat key as null-terminated. */
static VarEntry *vartable_get_or_create(VarTable *t, const char *key, size_t len)
{
    vartable_ensure_capacity(t);

    size_t klen = len;
    if (klen == 0) klen = strlen(key);
    char *kcopy = malloc(klen + 1);
    memcpy(kcopy, key, klen);
    kcopy[klen] = '\0';

    uint64_t h = hash_bytes(kcopy, klen);
    size_t idx = (size_t)(h & (t->capacity - 1));
    while (t->items[idx].key) {
        if (strcmp(t->items[idx].key, kcopy) == 0) {
            free(kcopy);
            return &t->items[idx];
        }
        idx = (idx + 1) & (t->capacity - 1);
    }

    t->items[idx].key = kcopy;
    t->items[idx].val = 0.0;
    t->count++;
    return &t->items[idx];
}

static VarEntry *vartable_get_or_create_ident(VarTable *t, Ident ident)
{
    return vartable_get_or_create(t, ident.chars, ident.len);
}

static VarEntry *vartable_find_ident(VarTable *t, Ident ident)
{
    return vartable_find(t, ident.chars, ident.len);
}

static void vartable_free(VarTable *t)
{
    if (!t || !t->items) return;
    for (size_t i = 0; i < t->capacity; ++i) {
        free(t->items[i].key);
    }
    free(t->items);
    t->items = NULL;
    t->capacity = 0;
    t->count = 0;
}


/* ---------- EvalExpr using the var table ---------- */
#if 0
double EvalExpr(Expr *expr)
{
    double result = 0.0;

    switch (expr->type)
    {
        case EXPR_NUMBER:
            result = expr->as.number;
            break;

        case EXPR_VARIABLE:
        {
            VarEntry *e = vartable_find_ident(expr->as.variable.ident);
            result = e ? e->val : 0.0;
        } break;

        case EXPR_BINOP:
        {
            BinNode bn = expr->as.binop;
            double rresult = EvalExpr(bn.rhs);

            if (bn.op == '=')
            {
                assert(bn.lhs->type == EXPR_VARIABLE && "Left-hand of assignment must be variable");
                VarEntry *dest = vartable_get_or_create_ident(bn.lhs->as.variable.ident);
                dest->val = rresult;
                result = rresult;
                break;
            }

            double lresult = EvalExpr(bn.lhs);

            switch (bn.op)
            {
                case '+': result = lresult + rresult; break;
                case '-': result = lresult - rresult; break;
                case '*': result = lresult * rresult; break;
                case '/': result = lresult / rresult; break;
                case '^': result = pow(lresult, rresult); break;
                default:
                    assert(!"TODO: unsupported operator");
            }
        } break;

        case EXPR_SEQUENCE:
        {
            ExprSeq *seq = &expr->as.seq;
            while (seq)
            {
                result = EvalExpr(seq->expr);
                seq = seq->next;
            }
        } break;

        case EXPR_PARSE_ERROR:
            assert(!"TODO: eval parse error");
            break;

        default:
            assert(0 && "Invalid code path!");
    }

    if (expr->flags & EXPR_FLAG_NEGATED)
        result = -result;

    return result;
}
#endif