#ifndef LAVA_LANG_INSTR_H_
#define LAVA_LANG_INSTR_H_

#include <string_view>
#include <cstdint>
#include <boost/container/small_vector.hpp>

namespace lava::lang {

#define LAVA_OPS \
  X(Nop) \
  X(Debug) \
  X(LdI32) \
  X(LdI64) \
  X(LdF32) \
  X(LdF64) \
  X(LdStr) \
  X(LdVar) \
  X(Eq) \
  X(Ne) \
  X(Lt) \
  X(Le) \
  X(Gt) \
  X(Ge) \
  X(Clz) \
  X(Ctz) \
  X(Popcount) \
  X(Add) \
  X(Sub) \
  X(Mul) \
  X(Div) \
  X(Rem) \
  X(And) \
  X(Or) \
  X(Xor) \
  X(Shl) \
  X(Shr) \
  X(Rotl) \
  X(Rotr) \
  X(Compl) \
  X(Not) \
  X(Neg) \
  X(Call) \
  X(Ret) \
  X(Jmp) \
  X(JmpIf) \

enum class Op {
#define X(OpName) OpName,
  LAVA_OPS
#undef X
};

std::string_view op_to_string(Op op);

struct LdI32Args {
  unsigned dest;
  uint32_t value;
};

struct LdI64Args {
  unsigned dest;
  uint64_t value;
};

struct LdF32Args {
  unsigned dest;
  float value;
};

struct LdF64Args {
  unsigned dest;
  double value;
};

struct LdStrArgs {
  unsigned dest;
  unsigned offset;
  unsigned size;
};

struct LdVarArgs {
  unsigned dest;
  unsigned offset;
  unsigned size;
};

struct UnaryArgs {
  unsigned dest;
  unsigned src;
};

struct BinaryArgs {
  unsigned dest;
  unsigned src[2];
};

struct CallArgs {
  unsigned fn;
  unsigned arg_count;
  unsigned *args;
};

struct ReturnArgs {
  unsigned value;
};

struct JumpArgs {
  unsigned bb;
};

struct JumpIfArgs {
  unsigned bb;
  unsigned bb_else;
  unsigned cond;
};

struct Instruction {
  Op op;
  union {
    LdI32Args ldi32;
    LdI64Args ldi64;
    LdF32Args ldf32;
    LdF64Args ldf64;
    LdStrArgs ldstr;
    LdVarArgs ldvar;
    UnaryArgs unary;
    BinaryArgs binary;
    CallArgs call;
    ReturnArgs ret;
    JumpArgs jmp;
    JumpIfArgs jmpif;
  };

  Instruction() : op{Op::Nop} {}
  Instruction(LdI32Args ldi32) : op{Op::LdI32}, ldi32{ldi32} {}
  Instruction(LdI64Args ldi64) : op{Op::LdI64}, ldi64{ldi64} {}
  Instruction(LdF32Args ldf32) : op{Op::LdF32}, ldf32{ldf32} {}
  Instruction(LdF64Args ldf64) : op{Op::LdF64}, ldf64{ldf64} {}
  Instruction(LdStrArgs ldstr) : op{Op::LdStr}, ldstr{ldstr} {}
  Instruction(LdVarArgs ldvar) : op{Op::LdVar}, ldvar{ldvar} {}
  Instruction(Op op, UnaryArgs unary) : op{op}, unary{unary} {}
  Instruction(Op op, BinaryArgs binary) : op{op}, binary{binary} {}
  Instruction(CallArgs call) : op{Op::Call}, call{call} {}
  Instruction(ReturnArgs ret) : op{Op::Ret}, ret{ret} {}
  Instruction(JumpArgs jmp) : op{Op::Jmp}, jmp{jmp} {}
  Instruction(JumpIfArgs jmpif) : op{Op::JmpIf}, jmpif{jmpif} {}

  Instruction(Instruction&&);
  Instruction &operator=(Instruction&&);
  ~Instruction();
};

std::string instr_to_string(const Instruction &instr);

} // namespace lava::lang

#endif /* LAVA_LANG_INSTR_H_ */
