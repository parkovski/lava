#include "lava/lang/iremit.h"
#include "lava/lang/instr.h"
#include <sstream>

using namespace lava::lang;

std::string_view lava::lang::op_to_string(Op op) {
  switch (op) {
  default: return "(invalid)";
#define X(OpName) case Op::OpName: return #OpName;
  LAVA_OPS
#undef X
  }
}

std::string lava::lang::instr_to_string(const Instruction &instr) {
  std::stringstream ss;

  ss << "  ";

  switch (instr.op) {
  case Op::LdI32:
    ss << "$" << instr.ldi32.dest << " = LdI32 " << instr.ldi32.value;
    break;

  case Op::LdI64:
    ss << "$" << instr.ldi64.dest << " = LdI64 " << instr.ldi32.value;
    break;

  case Op::LdF32:
    ss << "$" << instr.ldf32.dest << " = LdF32 " << instr.ldf32.value;
    break;

  case Op::LdF64:
    ss << "$" << instr.ldf64.dest << " = LdF64 " << instr.ldf64.value;
    break;

  case Op::LdStr:
    ss << "$" << instr.ldstr.dest << " = LdStr " << instr.ldstr.offset
      << ", " << instr.ldstr.size;

  case Op::LdVar:
    ss << "$" << instr.ldvar.dest << " = LdVar " << instr.ldvar.offset
      << ", " << instr.ldvar.size;
    break;

  case Op::Eq:
  case Op::Ne:
  case Op::Lt:
  case Op::Le:
  case Op::Gt:
  case Op::Ge:
  case Op::Add:
  case Op::Sub:
  case Op::Mul:
  case Op::Div:
  case Op::Rem:
  case Op::And:
  case Op::Or:
  case Op::Xor:
  case Op::Shl:
  case Op::Shr:
  case Op::Rotl:
  case Op::Rotr:
    ss << "$" << instr.binary.dest << " = " << op_to_string(instr.op)
      << " $" << instr.binary.src[0] << ", $" << instr.binary.src[1];
    break;

  case Op::Clz:
  case Op::Ctz:
  case Op::Popcount:
  case Op::Compl:
  case Op::Not:
  case Op::Neg:
    ss << "$" << instr.unary.dest << " = " << op_to_string(instr.op)
      << " $" << instr.unary.src;
    break;

  case Op::Ret:
    ss << "Ret $" << instr.ret.value;
    break;

  case Op::Jmp:
    ss << "Jmp #" << instr.jmp.bb;
    break;

  case Op::JmpIf:
    ss << "Jmp #" << instr.jmpif.bb << " if $" << instr.jmpif.cond;
    break;

  default:
    ss << op_to_string(instr.op);
    break;
  }

  return std::move(ss).str();
}

IREmitter::IREmitter(SymbolTable &symtab)
  : _symtab{&symtab}
  , _current_ns{&symtab.global_namespace()}
{}

void IREmitter::visit(const LiteralExpr &expr) {
  switch (expr.type()) {
  case LiteralType::Int:
    _current_reg = _current_fn->next_register();
    if (expr.int_value() <= UINT32_MAX) {
      _current_bb.instrs.emplace_back(Instruction{
        .op = Op::LdI32,
        .ldi32 = {
          .dest = _current_reg,
          .value = (uint32_t)expr.int_value(),
        }
      });
    } else {
      _current_bb.instrs.emplace_back(Instruction{
        .op = Op::LdI64,
        .ldi64 = {
          .dest = _current_reg,
          .value = expr.int_value(),
        }
      });
    }
    break;
  case LiteralType::Float:
    _current_reg = _current_fn->next_register();
    _current_bb.instrs.emplace_back(Instruction{
      .op = Op::LdF32,
      .ldf32 = {
        .dest = _current_reg,
        .value = expr.float_value(),
      }
    });
    break;
  case LiteralType::Double:
    _current_reg = _current_fn->next_register();
    _current_bb.instrs.emplace_back(Instruction{
      .op = Op::LdF64,
      .ldf64 = {
        .dest = _current_reg,
        .value = expr.double_value(),
      }
    });
    break;
  case LiteralType::String:
    {
      auto intern_string = _symtab->intern(expr.string_value());
      _current_reg = _current_fn->next_register();
      _current_bb.instrs.emplace_back(Instruction{
        .op = Op::LdStr,
        .ldstr = {
          .dest = _current_reg,
          .offset = (unsigned)intern_string.offset,
          .size = (unsigned)intern_string.size,
        },
      });
    }
    break;
  }
}

void IREmitter::visit(const IdentExpr &expr) {
  _current_reg = _current_fn->next_register();
  auto intern_string = _symtab->intern(expr.value());
  _current_bb.instrs.emplace_back(Instruction{
    .op = Op::LdVar,
    .ldvar = {
      .dest = _current_reg,
      .offset = (unsigned)intern_string.offset,
      .size = (unsigned)intern_string.size,
    },
  });
}

void IREmitter::visit(const PrefixExpr &expr) {
  NodeVisitor::visit(expr);
  auto expr_reg = _current_reg;

  Op op;
  switch (expr.op()) {
  case TkComma:
    return;
  case TkDotDot:
    throw std::runtime_error{"Not supported: .."};
  case TkTilde:
    op = Op::Compl;
    break;
  case TkExcl:
    op = Op::Not;
    break;
  case TkMinus:
    op = Op::Neg;
    break;
  case TkPlus:
    return;
  case TkStar:
  case TkStarStar:
  case TkAnd:
    throw std::runtime_error{"Not supported: *, &"};
  case TkMinusMinus:
  case TkPlusPlus:
    throw std::runtime_error{"Not supported: --, ++"};
  case TkDot:
    throw std::runtime_error{"Not supported: . (prefix)"};
  case TkReturn:
    _current_bb.instrs.emplace_back(Instruction{
      .op = Op::Ret,
      .ret = { .value = _current_reg }
    });
    return;
  }

  _current_reg = _current_fn->next_register();
  _current_bb.instrs.emplace_back(Instruction{
    .op = op,
    .unary = {
      .dest = _current_reg,
      .src = expr_reg,
    },
  });
}

void IREmitter::visit(const PostfixExpr &expr) {
  NodeVisitor::visit(*expr.expr());

  switch (expr.op()) {
  case TkComma:
  case TkDotDot:
  case TkMinusMinus:
  case TkPlusPlus:
  case TkExcl:
  case TkQuestion:
    throw std::runtime_error{"postfix not supported"};
  }
}

void IREmitter::visit(const BinaryExpr &expr) {
  NodeVisitor::visit(*expr.left());
  auto left_reg = _current_reg;
  NodeVisitor::visit(*expr.right());
  auto right_reg = _current_reg;

  Op op;

  switch (expr.op()) {
  default:
    throw std::runtime_error{"not supported binary op"};
  case TkEqEq:
    op = Op::Eq;
    break;
  case TkExclEq:
    op = Op::Ne;
    break;
  case TkLess:
    op = Op::Lt;
    break;
  case TkLessEq:
    op = Op::Le;
    break;
  case TkGreater:
    op = Op::Gt;
    break;
  case TkGreaterEq:
    op = Op::Ge;
    break;
  case TkAnd:
    op = Op::And;
    break;
  case TkOr:
    op = Op::Or;
    break;
  case TkHat:
    op = Op::Xor;
    break;
  case TkLessLess:
    op = Op::Shl;
    break;
  case TkLessMinusLess:
    op = Op::Rotl;
    break;
  case TkGreaterGreater:
    op = Op::Shr;
    break;
  case TkGreaterMinusGreater:
    op = Op::Rotr;
    break;
  case TkMinus:
    op = Op::Sub;
    break;
  case TkPlus:
    op = Op::Add;
    break;
  case TkPercent:
    op = Op::Rem;
    break;
  case TkStar:
    op = Op::Mul;
    break;
  case TkSlash:
    op = Op::Div;
    break;
  case TkDot:
    op = Op::Nop;
    break;
  }

  _current_reg = _current_fn->next_register();
  _current_bb.instrs.emplace_back(Instruction{
    .op = op,
    .binary = {
      .dest = _current_reg,
      .src = { left_reg, right_reg }
    },
  });
}

void IREmitter::visit(const InvokeExpr &expr) {
  _current_bb.instrs.emplace_back(Instruction{
    .op = Op::Call,
    .call = { .fn = 0 }
  });
}

void IREmitter::visit(const ScopeExpr &expr) {
  NodeVisitor::visit(expr);
}

void IREmitter::visit(const FunDefItem &item) {
  auto *sym = _current_ns->get(_symtab->intern(item.name()));
  if (!sym) {
    throw std::runtime_error{"Undefined symbol"};
  }

  auto *fn = dynamic_cast<Function*>(sym);
  if (!fn) {
    throw std::runtime_error{"Symbol not a function"};
  }

  _current_fn = fn;
  auto *prev_ns = _current_ns;
  _current_ns = &fn->locals_namespace();
  NodeVisitor::visit(item);
  if (!_current_bb.instrs.empty()) {
    _current_fn->push_basicblock(std::move(_current_bb));
  }
  _current_ns = prev_ns;
  _current_fn = nullptr;
}
