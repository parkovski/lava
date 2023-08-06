#ifndef LAVA_SYN_SYMTAB_H_
#define LAVA_SYN_SYMTAB_H_

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <string_view>
#include <memory>
#include "symbol.h"
#include "types.h"
#include "instr.h"

namespace lava::syn {

namespace detail {
  struct ptrhash_type {
    const size_t operator()(const std::unique_ptr<Type> &value) const {
      return std::hash<Type>{}(*value);
    }
  };

  struct ptreq_type {
    const bool operator()(const std::unique_ptr<Type> &a,
                          const std::unique_ptr<Type> &b) const {
      return *a == *b;
    }
  };
}

struct SymbolTable {
private:
  std::unordered_set<std::unique_ptr<Type>,
                     detail::ptrhash_type, detail::ptreq_type>
    _unnamed_types;
  Namespace _global;

  void enter_initial_types();

public:
  explicit SymbolTable();
  ~SymbolTable();

  // Takes ownership of `type`, either deleting it or inserting it into the
  // global type table.
  Type *find_unnamed_type(std::unique_ptr<Type> type);

  Namespace &global_ns() { return _global; }
  const Namespace &global_ns() const { return _global; }
};

} // namespace lava::syn

#endif // LAVA_SYN_SYMTAB_H_
