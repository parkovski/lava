#ifndef LAVA_SYN_INSTR_H_
#define LAVA_SYN_INSTR_H_

#include <boost/container/small_vector.hpp>
#include <cstdint>

namespace lava::syn {

enum class Op {
  Nop,
  Compl,
  Not,
  Neg,
  Pos,
  Assign,
  Call,
  Scope,
};

struct Symbol;
union Value {
  uint64_t u64;
  uint32_t u32;
  double f64;
  float f32;
  void *p;
  Symbol *s;
};

enum class ValueType {
  U64,
  U32,
  F64,
  F32,
  Pointer,
  Symbol,
};

struct TypedValue {
  ValueType type;
  Value value;
};

struct Instruction {
  Op op;
  boost::container::small_vector<Value, 3> args;
};

} // namespace lava::syn

#endif // LAVA_SYN_INSTR_H_
