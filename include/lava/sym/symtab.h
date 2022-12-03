#ifndef LAVA_SYM_SYMTAB_H_
#define LAVA_SYM_SYMTAB_H_

#include "lava/data/arena.h"

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <span>
#include <string_view>

namespace lava::sym {

struct meminfo32 {
  uint32_t size       : 29;
  uint32_t alignshift : 3;

  constexpr uint32_t align() const noexcept
  { return UINT32_C(1) << alignshift; }
};

struct meminfo64 {
  uint64_t size       : 59;
  uint64_t alignshift : 5;

  constexpr uint64_t align() const noexcept
  { return UINT64_C(1) << alignshift; }
};

#if SIZE_MAX == UINT32_MAX
typedef meminfo32 meminfosz;
#elif SIZE_MAX == UINT64_MAX
typedef meminfo64 meminfosz;
#else
# error "Unsupported size_t size"
#endif

enum ID : uint32_t {
  ID_undefined = uint32_t(-1),
  ID_root = 0,
  ID_meminfo32,
  ID_meminfo64,
  ID_meminfosz,
  ID_scopemap,
};

// Interned constant reference.
struct CRef {
  size_t size = 0;
  const char *p = nullptr;

  friend bool operator==(const CRef &, const CRef &) = default;

  operator std::string_view() const noexcept
  { return std::string_view{p, size}; }

  constexpr explicit operator bool() const noexcept
  { return p != nullptr; }
};

// Maps name to pair of [id, index].
typedef std::unordered_map<CRef, std::pair<uint32_t, uint32_t>> scopemap;

struct Symtab {
  Symtab();
  ~Symtab();

  // Find an interned data block.
  CRef find_data(const void *p, size_t size) const noexcept;

  // Intern a chunk of data.
  CRef intern_data(const void *p, size_t align, size_t size);

  // Find an interned string (equivalent to
  // `find_data(str.data(), str.size())`).
  CRef find_str(std::string_view str) const noexcept;

  // Intern a string (equivalent to `intern_data(str.data(), 1, str.size())`).
  CRef intern_str(std::string_view str);

  // Get the ID of a named symbol.
  uint32_t id(uint32_t parent, CRef name) const noexcept;

  uint32_t id(uint32_t parent, std::string_view name) const noexcept;

  uint32_t parent(uint32_t id) const noexcept;

  uint32_t mksym(uint32_t parent, CRef name);

  void *get_attr(uint32_t id, uint32_t attr_id) noexcept;

  const void *get_attr(uint32_t id, uint32_t attr_id) const noexcept
  { return const_cast<Symtab*>(this)->get_attr(id, attr_id); }

  template<class T>
  T *get_attr(uint32_t id, uint32_t attr_id) noexcept
  { return static_cast<T*>(get_attr(id, attr_id)); }

  template<class T>
  const T *get_attr(uint32_t id, uint32_t attr_id) const noexcept
  { return const_cast<Symtab*>(this)->get_attr<T>(id, attr_id); }

  std::pair<void *, bool> get_or_put_attr(uint32_t id, uint32_t attr_id);

  template<class T>
  std::pair<T*, bool> get_or_put_attr(uint32_t id, uint32_t attr_id) {
    auto [p, inserted] = get_or_put_attr(id, attr_id);
    return std::make_pair(static_cast<T*>(p), inserted);
  }

private:
  struct SymbolData {
    uint32_t id;
    uint32_t parent;
    CRef name;
    std::vector<std::pair<uint32_t, void*>> attrs;
  };

  lava_arena _arena;
  std::unordered_set<std::string_view> _intern_data;
  std::vector<SymbolData> _symbol_data;
};

} // namespace lava::sym

namespace std {
  template<>
  struct hash<lava::sym::CRef> {
    size_t operator()(const lava::sym::CRef &c) const noexcept {
      return hash<const void*>{}(c.p);
    }
  };
}

#endif /* LAVA_SYM_SYMTAB_H_ */
