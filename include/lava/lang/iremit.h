#ifndef LAVA_LANG_IREMIT_H_
#define LAVA_LANG_IREMIT_H_

#include "lava/lang/nodes.h"
#include "visitor.h"
#include "symbol.h"

namespace lava::lang {

struct IREmitter : NodeVisitor {
  SymbolTable *_symtab;
  Namespace *_current_ns;
  Function *_current_fn = nullptr;
  BasicBlock _current_bb;
  unsigned _current_reg = 0;
  unsigned _current_continue = 0;

  IREmitter(SymbolTable &symtab);

  void visit(const LiteralExpr &expr) override;
  void visit(const IdentExpr &expr) override;
  void visit(const PrefixExpr &expr) override;
  void visit(const PostfixExpr &expr) override;
  void visit(const BinaryExpr &expr) override;
  void visit(const InvokeExpr &expr) override;
  void visit(const ScopeExpr &expr) override;
  void visit(const ReturnExpr &expr) override;
  void visit(const IfExpr &expr) override;
  void visit(const WhileExpr &expr) override;
  void visit(const LoopExpr &expr) override;
  void visit(const BreakContinueExpr &expr) override;

  void fix_breaks(unsigned from, unsigned to);
  bool simplify_jumps();

  void visit(const FunDefItem &item) override;
};

} // namespace lava::lang

#endif /* LAVA_LANG_IREMIT_H_ */
