#if defined(__linux__)
# define ALLOC_POSIX
#elif defined(__APPLE__)
# define ALLOC_POSIX
#elif defined(BSD)
# define ALLOC_POSIX
#elif defined(_WIN32)
# define ALLOC_WIN32
#endif

#if defined(ALLOC_POSIX)
# define _POSIX_C_SOURCE 200809L
# include <stdlib.h>
# include <unistd.h>
#elif defined(ALLOC_WIN32)
# include <Windows.h>
#else
# error "Unknown OS allocation API"
#endif

#include <stdbool.h>
#include <assert.h>

#include "lava/data/arena.h"

// OS support {{{

// Get the system's default page size.
static inline unsigned get_page_size(void) {
#if defined(ALLOC_POSIX)
# if defined(_SC_PAGESIZE)
  return (unsigned)sysconf(_SC_PAGESIZE);
# elif defined(_SC_PAGE_SIZE)
  return (unsigned)sysconf(_SC_PAGE_SIZE);
# else
# warning "Missing _SC_PAGESIZE and _SC_PAGE_SIZE, assuming 4kib"
  return 4096U;
# endif
#elif defined(ALLOC_WIN32)
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwPageSize;
#endif
}

// Allocate `count` pages of size `size`.
// @param size System memory page size.
// @param count Number of pages to allocate.
// @return Pointer to the newly allocated pages, or `NULL` if allocation
// failed.
static inline void *alloc_pages(size_t size, size_t count) {
  void *p;
#if defined(ALLOC_POSIX)
  // Returns 0 on success or an error value:
  // EINVAL = invalid alignment
  // ENOMEM = out of memory
  if (posix_memalign(&p, size, size * count)) {
    return NULL;
  }
#elif defined(ALLOC_WIN32)
  // Returns NULL on error, see GetLastError().
  p = VirtualAlloc(NULL, size * count, MEM_COMMIT | MEM_RESERVE,
                   PAGE_READWRITE);
#endif
  return p;
}

#if defined(ALLOC_POSIX)
# define free_pages free
#elif defined(ALLOC_WIN32)
# define free_pages(p) VirtualFree(p, 0, MEM_RELEASE)
#endif

// }}} OS support

// Private API {{{

/// Add `count` pages to the arena.
/// @param arena The arena tracking the pages.
/// @param count Number of pages to add.
/// @return `true` if pages were added, else `false`.
static bool add_pages(lava_arena *arena, size_t count) {
  void *p;

  if (arena->count == arena->capacity) {
    uint32_t cap;
    if (arena->capacity == 0) {
      cap = 128;
    } else {
      cap = arena->capacity * 2;
    }

    void *info = realloc(arena->info, cap * sizeof(lava_arena_alloc_info));
    if (!info) {
      return false;
    }
    arena->info = info;
    arena->capacity = cap;
  }

  p = alloc_pages(arena->pagesize, count);
  if (!p) {
    return false;
  }

  arena->info[arena->count++] = (lava_arena_alloc_info) {
    .p = p,
    .pages = count,
    .offset = 0,
  };
  return true;
}

// }}}

void lava_arena_init(lava_arena *arena) {
  *arena = (lava_arena) {
    .info     = NULL,
    .count    = 0,
    .capacity = 0,
    .pagesize = get_page_size(),
  };
}

void lava_arena_fini(lava_arena *arena) {
  for (uint32_t i = arena->count; i != 0; --i) {
    free_pages(arena->info[i-1].p);
  }
}

void *lava_arena_alloc(lava_arena *arena, size_t align, size_t size) {
#if defined(__has_builtin) && __has_builtin(__builtin_popcount)
  assert(__builtin_popcount(align) == 1);
#endif

  lava_arena_alloc_info *info;
  size_t start, end, allocated;
  if (arena->capacity > 0) {
    info = &arena->info[arena->count - 1];
    start = (info->offset + align - 1) & -align;
    end = start + size;
    allocated = info->pages * arena->pagesize;
  } else {
    // Initial state - no capacity.
    end = size;
    allocated = 0;
  }

  if (end > allocated) {
    assert(align <= arena->pagesize && "Large alignments not supported");
    size_t npages = (size + arena->pagesize - 1) / arena->pagesize;
    assert(npages < UINT32_MAX);
    if (!add_pages(arena, npages)) {
      return NULL;
    }
    info = &arena->info[arena->count - 1];
    start = 0;
    end = size;
  }

  void *p = (char *)info->p + start;
  info->offset = end;
  return p;
}
