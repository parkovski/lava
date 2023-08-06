#ifndef LAVA_SYN_INSTR_H_
#define LAVA_SYN_INSTR_H_

#include <boost/container/small_vector.hpp>
#include <cstdint>

namespace lava::syn {

enum class Op {
  Nop,
  Const,
  FieldRef,
  Eq,
  Ne,
  Lt,
  Gt,
  Le,
  Ge,
  Clz,
  Ctz,
  Popcnt,
  Add,
  Sub,
  Mul,
  Div,
  Rem,
  And,
  Or,
  Xor,
  Shl,
  Shr,
  Rotl,
  Rotr,
  Compl,
  Not,
  Neg,
  Pos,
  Goto,
  If,
  Loop,
  Assign,
  Call,
  Return,
  PushScope,
};

struct Symbol;
union Value {
  uint64_t u64;
  int64_t  i64;
  uint32_t u32;
  int32_t  i32;
  double   f64;
  float    f32;
  const char *str;
  void    *ptr;
  Symbol  *sym;
};

enum class ValueType {
  U64,
  I64,
  U32,
  I32,
  F64,
  F32,
  String,
  Pointer,
  Symbol,
};

struct TypedValue {
  Value value;
  uint32_t size;
  ValueType type;

  TypedValue() noexcept
    : value{.u64 = 0}
    , size{0}
    , type{ValueType::U64}
  {}

#define DECLARE_CONSTRUCTOR(Type, Field, VType) \
  TypedValue(Type value) noexcept \
    : value{.Field = value} \
    , size{1} \
    , type{ValueType::VType} \
  {}

  DECLARE_CONSTRUCTOR(uint64_t, u64, U64)
  DECLARE_CONSTRUCTOR(int64_t, i64, I64)
  DECLARE_CONSTRUCTOR(uint32_t, u32, U32)
  DECLARE_CONSTRUCTOR(int32_t, i32, I32)
  DECLARE_CONSTRUCTOR(double, f64, F64)
  DECLARE_CONSTRUCTOR(float, f32, F32)
  DECLARE_CONSTRUCTOR(Symbol*, sym, Symbol)

#undef DECLARE_CONSTRUCTOR

  TypedValue(std::string_view str) noexcept
    : value{.str = str.data()}
    , size{(uint32_t)str.size()}
    , type{ValueType::String}
  {}
};

struct Instruction {
  Op op;
  boost::container::small_vector<TypedValue, 3> args;
};

} // namespace lava::syn

#endif // LAVA_SYN_INSTR_H_
