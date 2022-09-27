#ifndef ASH_SYM_SYM_H_
#define ASH_SYM_SYM_H_

#include <cstdint>

namespace ash::sym {

typedef uint32_t id_t;
constexpr id_t id_undef = -1U;

namespace attr {
  template<class T> struct type_id;
  template<id_t Id> struct id_type;

  template<class T>
  constexpr id_t type_id_v = type_id<T>::value;

  template<id_t Id>
  using id_type_t = typename id_type<Id>::type;
}

enum class CoreId : id_t {
  Undefined = id_undef,
  AttrList = 0,
  Provides,
  Requires,
  Instance,
  MemoryInfo,
  Indexable,
  IndexRef,
  ArrayInfo,
  TupleInfo,
  FunctionInfo,
  Namespace,
  Value,
};

} // namespace ash::sym

#endif /* ASH_SYM_SYM_H_ */
