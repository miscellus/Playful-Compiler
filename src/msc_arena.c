#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#error TODO
#endif

#include "msc_arena.h"

msc_arena msc_arena_create(size_t cap)
{
    msc_arena arena;
    memset(&arena, 0, sizeof(arena));

#ifdef _WIN32
    arena.base = (uintptr_t)VirtualAlloc(NULL, cap, MEM_RESERVE, PAGE_READWRITE);
#else
#error TODO(jkkammersgaard): implement msc_arena_create on this platform
#endif
    arena.at = arena.base;
    arena.committed_end = arena.base;
    arena.reserved_end = arena.base + cap;
    return arena;
}

void *msc_arena_push_size(msc_arena *arena, size_t size, size_t align) {

    uintptr_t alloc_base = (arena->flags & MSC_ARENA_OPT_NO_ALIGN)
        ? arena->at
        : msc_align_ptr(arena->at, align);

    uintptr_t alloc_end = alloc_base + size;

    if (alloc_end >= arena->committed_end) {
        // TODO(jkkammersgaard): @hardcode, get page size from OS
        uintptr_t new_committed_end = msc_align_ptr(alloc_end, 4096);
#ifdef _WIN32
        if (!VirtualAlloc((void *)arena->base, new_committed_end - arena->base, MEM_COMMIT, PAGE_READWRITE)) {
            return NULL;
        }
        arena->committed_end = new_committed_end;
#else
#error TODO(jkkammersgaard): implement msc_arena_create on this platform
#endif
    }

    if (alloc_end >= arena->reserved_end) {
        return NULL;
    }

    arena->at = alloc_end;

    return (void *)alloc_base;
}

uintptr_t msc_align_ptr(uintptr_t ptr, size_t align) {
    if (msc_is_pow2(align)) {
        return (ptr + align - 1) & ~(align - 1);
    }
    return ((ptr + align - 1) / align) * align;
}

void msc_arena_reset(msc_arena *arena) {
    arena->at = arena->base;
}
