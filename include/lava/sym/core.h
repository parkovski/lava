#ifndef LAVA_SYM_CORE_H_
#define LAVA_SYM_CORE_H_

#include <cstdint>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace lava::sym {

enum ID : uint32_t {
  ID_undefined = uint32_t(-1),
  ID_root = 0,
  ID_never,
  ID_void,
  ID_null,
  ID_MemInfo32,
  ID_MemInfo64,
  ID_ScopeMap,
  ID_Symbol,
  ID_prototype,
  ID_GenericFn,
  ID_ContextFn,
  ID_init,
  ID_fini,
};

// Interned constant reference.
struct InternRef {
  size_t size = 0;
  const char *p = nullptr;

  friend bool operator==(const InternRef &a, const InternRef &b) noexcept
  { return a.p == b.p; }

  friend auto operator<=>(const InternRef &a, const InternRef &b) noexcept
  { return a.p <=> b.p; }

  operator std::string_view() const noexcept
  { return std::string_view{p, size}; }

  constexpr explicit operator bool() const noexcept
  { return p != nullptr; }
};

struct MemInfo32 {
  uint32_t _size       : 29;
  uint32_t _alignshift : 3;

  constexpr uint32_t size() const noexcept
  { return _size; }

  constexpr uint32_t align() const noexcept
  { return UINT32_C(1) << _alignshift; }
};

struct MemInfo64 {
  uint64_t _size       : 59;
  uint64_t _alignshift : 5;

  constexpr uint64_t size() const noexcept
  { return _size; }

  constexpr uint64_t align() const noexcept
  { return UINT64_C(1) << _alignshift; }
};

#if SIZE_MAX == UINT32_MAX
typedef MemInfo32 MemInfoSz;
#elif SIZE_MAX == UINT64_MAX
typedef MemInfo64 MemInfoSz;
#else
# error "Unsupported size_t size"
#endif

// List of attribute IDs used for requires and inherits lists.
typedef std::vector<uint32_t> IdList;

// List of attribute instances used for provides and implements lists.
typedef std::vector<std::pair<uint32_t, void*>> InstanceList;

// Maps name to pair of [id, index].
typedef std::unordered_map<InternRef, std::pair<uint32_t, uint32_t>> ScopeMap;

typedef void (*GenericFn)(void *data);

struct ContextFn {
  void (*fn)(void *ctx, void *data);
  void *ctx;
};

struct Symbol {
  uint32_t     id;
  uint32_t     parent;
  InternRef    name;
  IdList       metas;
  InstanceList attrs;
};

} // namespace lava::sym

namespace std {
  template<>
  struct hash<lava::sym::InternRef> {
    size_t operator()(const lava::sym::InternRef &r) const noexcept {
      return hash<const void*>{}(r.p);
    }
  };
}

#endif /* LAVA_SYM_CORE_H_ */
