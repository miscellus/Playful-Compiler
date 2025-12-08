#ifndef MISCELLUS_ARENA
#define MISCELLUS_ARENA

#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <stdalign.h>

#ifndef msc_is_pow2
#define msc_is_pow2(v) (0 == ((v) & ((v) - 1)))
#endif

#ifndef msc_offsetof
#define msc_offsetof(T, M) ((size_t)(&((T*)0)->M))
#endif

#ifndef msc_alignof
#ifdef __cplusplus
template<typename T> struct msc_alignment_trick { char c; T member; };
#define msc_alignof(T) msc_offsetof(msc_alignment_trick<T>, member)
#else
#define msc_alignof(T) (8)
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
    MSC_ARENA_OPT_NO_ALIGN = (1 << 0),
    MSC_ARENA_OPT_ZERO = (1 << 1),
};

typedef struct msc_arena_t {
    uintptr_t base;
    uintptr_t at;
    uintptr_t committed_end;
    uintptr_t reserved_end;
    uint32_t flags;
} msc_arena;

msc_arena msc_arena_create(size_t cap);
uintptr_t msc_align_ptr(uintptr_t ptr, size_t align);
void *msc_arena_push_size(msc_arena *arena, size_t size, size_t align);
void msc_arena_reset(msc_arena *arena);

#define msc_arena_push(A, T) ((T *)msc_arena_push_size((A), sizeof(T), msc_alignof(T)))
#define msc_arena_push_array(A, T, N) ((T *)msc_arena_push_size((A), (N)*sizeof(T), msc_alignof(T)))

#ifdef __cplusplus
}
#endif

#endif // MISCELLUS_ARENA
