#ifndef ASH_SYM_ATTR_H_
#define ASH_SYM_ATTR_H_

#include "sym.h"

#include <string_view>
#include <unordered_map>
#include <boost/container/small_vector.hpp>

namespace ash::sym {

class SymbolTable;

namespace attr {

#define ASH_TYPE_ID(Type, Id) \
  template<> struct id_type<Id> { using type = Type; }; \
  template<> struct type_id<Type> { constexpr static id_t value = Id; }

#define ASH_CORE_STRUCT(Name) \
  struct Name; \
  ASH_TYPE_ID(Name, static_cast<id_t>(CoreId::Name)); \
  struct Name

template<class T = void>
struct AttrRef {
  SymbolTable *symtab;
  T *attr;
};

template<class T = void>
struct TypedInstance {
  T *inst;
  id_t type;

  template<class U>
  TypedInstance<U> as() const noexcept {
    return {static_cast<U*>(inst), type};
  }
};

struct IdList {
  boost::container::small_vector<id_t, 4> ids;
};

struct InstanceList {
  boost::container::small_vector<TypedInstance<>, 4> instances;
};

ASH_CORE_STRUCT(AttrList) : InstanceList {};

// Instantiations of the symbol have these properties.
ASH_CORE_STRUCT(Provides) : InstanceList {};

// To instantiate the symbol, you must provide these.
ASH_CORE_STRUCT(Requires) : IdList {};

// The symbol uses the provides/requires lists of the symbols in this list.
ASH_CORE_STRUCT(Instance) : IdList {};

ASH_CORE_STRUCT(MemoryInfo) {
  uint32_t align;
  size_t size;
};

ASH_CORE_STRUCT(Indexable) {};

ASH_CORE_STRUCT(IndexRef) {
  size_t index;
};

// Provides MemoryInfo, Indexable
ASH_CORE_STRUCT(ArrayInfo) {
  id_t type;
  size_t count;
};

// Provides MemoryInfo
ASH_CORE_STRUCT(TupleInfo) : IdList {};

ASH_CORE_STRUCT(FunctionInfo) {
  id_t rettype;
  id_t argtype;
};

ASH_CORE_STRUCT(Namespace) {
  id_t get_name(std::string_view name) const {
    auto it = names.find(static_cast<const void *>(name.data()));
    if (it == names.end()) {
      return id_undef;
    }
    return it->second;
  }

  // Returns true if name maps to id. Only returns false when `replace` is
  // false and a name mapping already exists.
  bool set_name(std::string_view name, id_t id, bool replace = false) {
    if (replace) {
      return names
        .insert_or_assign(static_cast<const void *>(name.data()), id)
        .second;
    } else {
      auto it = names
        .insert(std::make_pair(static_cast<const void *>(name.data()), id));
      return it.second || id == it.first->second;
    }
  }

private:
  std::unordered_map<const void *, id_t> names;
};

ASH_CORE_STRUCT(Value) {
  id_t type;
  union {
    size_t u;
    ptrdiff_t i;
    void *p;
  };
};

} // namespace attr
} // namespace ash::sym

#endif /* ASH_SYM_ATTR_H_ */
