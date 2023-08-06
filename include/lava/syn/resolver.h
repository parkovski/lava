#ifndef LAVA_SYN_RESOLVER_H_
#define LAVA_SYN_RESOLVER_H_

#include "nodes.h"
#include "symtab.h"

namespace lava::syn {

struct Resolver {
private:
  SymbolTable &_symtab;
  Scope *_scope;

public:
  explicit Resolver(SymbolTable &symtab) noexcept
    : _symtab{symtab}
    , _scope{nullptr}
  {}

  void resolve_document(const Document &doc);

  void resolve_item(const Item &item);
  void resolve_item(const ExprItem &item);
  void resolve_item(const VarDeclItem &item);
  void resolve_item(const FunDeclItem &item);
  void resolve_item(const FunDefItem &item);

  void resolve_expr(const Expr &expr);
  void resolve_expr(const LiteralExpr &expr);
  void resolve_expr(const IdentExpr &expr);
  void resolve_expr(const PrefixExpr &expr);
  void resolve_expr(const PostfixExpr &expr);
  void resolve_expr(const BinaryExpr &expr);
  void resolve_expr(const ParenExpr &expr);
  void resolve_expr(const InvokeExpr &expr);
  void resolve_expr(const ScopeExpr &expr);

  Type *resolve_type(Scope *scope, const Expr &expr);
  Type *resolve_type(Scope *scope, const IdentExpr &expr);
  Type *resolve_type(Scope *scope, const BinaryExpr &expr);

  Scope *resolve_scope(Scope *scope, const Expr &expr);
  Scope *resolve_scope(Scope *scope, const IdentExpr &expr);
  Scope *resolve_scope(Scope *scope, const BinaryExpr &expr);
};

} // namespace lava::syn

#endif // LAVA_SYN_RESOLVER_H_
