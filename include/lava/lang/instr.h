#ifndef LAVA_LANG_INSTR_H_
#define LAVA_LANG_INSTR_H_

namespace lava::lang {

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

} // namespace lava::lang

#endif /* LAVA_LANG_INSTR_H_ */
