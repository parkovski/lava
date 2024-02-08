#ifndef LAVA_LANG_VISITOR_H_
#define LAVA_LANG_VISITOR_H_

#include "nodes.h"

namespace lava::lang {

struct NodeVisitor {
  virtual void visit(const Document &doc);

  virtual void visit(const Expr &expr);
  virtual void visit(const LiteralExpr &expr);
  virtual void visit(const IdentExpr &expr);
  virtual void visit(const PrefixExpr &expr);
  virtual void visit(const PostfixExpr &expr);
  virtual void visit(const BinaryExpr &expr);
  virtual void visit(const ParenExpr &expr);
  virtual void visit(const InvokeExpr &expr);
  virtual void visit(const ScopeExpr &expr);
  virtual void visit(const ReturnExpr &expr);
  virtual void visit(const IfExpr &expr);
  virtual void visit(const WhileExpr &expr);
  virtual void visit(const LoopExpr &expr);
  virtual void visit(const BreakContinueExpr &expr);

  virtual void visit(const Item &item);
  virtual void visit(const EmptyItem &item);
  virtual void visit(const ExprItem &item);
  virtual void visit(const VarDeclItem &item);
  virtual void visit(const FunDeclItem &item);
  virtual void visit(const FunDefItem &item);
  virtual void visit(const StructDefItem &item);

  virtual void visit(const VarDecl &var);
  virtual void visit(const ArgDecl &arg);
};

} // namespace lava::lang

#endif /* LAVA_LANG_VISITOR_H_ */
