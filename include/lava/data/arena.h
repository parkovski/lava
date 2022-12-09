#ifndef LAVA_ARENA_H_
#define LAVA_ARENA_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Arena allocation info
typedef struct {
  // Pointer to allocated memory.
  void *p;

  // Amount of claimed memory. `(char*)p + offset` points to the first
  // unclaimed byte in the page.
  uint32_t offset;

  // Number of pages pointed to by `p`.
  uint32_t pages;
} lava_arena_alloc_info;

// Arena allocator. Reserves memory in pages from the OS and allocates using a
// simple pointer bump. No memory is freed until the arena is deinitialized by
// calling `lava_arena_fini`.
typedef struct {
  // Array of allocation descriptors.
  lava_arena_alloc_info *info;

  // Number of initialized allocation descriptors.
  uint32_t               count;

  // Array size of `info` including uninitialized elements.
  uint32_t               capacity;

  // OS default memory page size.
  uint32_t               pagesize;
} lava_arena;

/// Initialize the arena.
/// @param arena The arena.
void lava_arena_init(lava_arena *arena);

/// Free all pages held by the arena.
/// @param arena The arena.
void lava_arena_fini(lava_arena *arena);

/// Allocate memory within the arena using a pointer bump strategy if possible,
/// otherwise adding pages to the arena. The memory is held until the arena is
/// deinitialized by calling `lava_arena_fini`.
/// @param arena The arena.
/// @param align Alignment of the desired allocation.
/// @param size Size of the desired allocation.
/// @return Pointer to the new allocation.
void *lava_arena_alloc(lava_arena *arena, size_t align, size_t size);

#ifdef __cplusplus
} /* extern "C" */

#include <new>
#include <limits>
#include <type_traits>
#include <utility>

namespace lava::data {

/// C++ allocator adapter for @see `lava_arena`.
template<class T>
struct arena_allocator {
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::true_type;

  arena_allocator(lava_arena *arena) noexcept
    : _arena{arena}
  {}

  arena_allocator(const arena_allocator &) noexcept = default;

  template<class U>
  arena_allocator(const arena_allocator<U> &other) noexcept
    : _arena{other._arena}
  {}

  bool operator==(const arena_allocator &) const & noexcept = default;

  [[nodiscard]] T *allocate(size_t n) {
    if (std::numeric_limits<size_t>::max() / sizeof(T) < n) {
      throw std::bad_array_new_length{};
    }
    T *p = static_cast<T*>(
      ::lava_arena_alloc(_arena, alignof(T), n * sizeof(T))
    );
    if (!p) {
      throw std::bad_alloc{};
    }
    return p;
  }

  void deallocate(T *, size_t) {}

  template<class U>
  void construct(U *ptr) noexcept(std::is_nothrow_default_constructible_v<U>) {
    ::new(static_cast<void*>(ptr)) U;
  }

  template<class U, class... Args>
  void construct(U *ptr, Args &&...args)
    noexcept(std::is_nothrow_constructible_v<U, Args...>) {
    ::new(static_cast<void*>(ptr)) U(std::forward<Args>(args)...);
  }

private:
  lava_arena *_arena;
};

} // namespace lava::data

#endif // __cplusplus

#endif /* LAVA_ARENA_H_ */
