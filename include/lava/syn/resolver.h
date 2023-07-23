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
    , _scope{&symtab.global_scope()}
  {}

  void resolve(const Document &doc);

  void resolve(const Item &item);
  void resolve(const ExprItem &item);
  void resolve(const VarDeclItem &item);
  void resolve(const FunDeclItem &item);
  void resolve(const FunDefItem &item);

  void resolve(const Expr &expr);
  void resolve(const LiteralExpr &expr);
  void resolve(const IdentExpr &expr);
  void resolve(const PrefixExpr &expr);
  void resolve(const PostfixExpr &expr);
  void resolve(const BinaryExpr &expr);
  void resolve(const ParenExpr &expr);
  void resolve(const InvokeExpr &expr);
  void resolve(const ScopeExpr &expr);

  Type *resolve_type(Scope *scope, const Expr &expr);
  Type *resolve_type(Scope *scope, const IdentExpr &expr);
  Type *resolve_type(Scope *scope, const BinaryExpr &expr);

  Scope *resolve_scope(Scope *scope, const Expr &expr);
  Scope *resolve_scope(Scope *scope, const IdentExpr &expr);
  Scope *resolve_scope(Scope *scope, const BinaryExpr &expr);
};

} // namespace lava::syn

#endif // LAVA_SYN_RESOLVER_H_
