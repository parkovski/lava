#ifndef LAVA_LANG_FIRSTPASS_H_
#define LAVA_LANG_FIRSTPASS_H_

#include "visitor.h"
#include "symbol.h"

namespace lava::lang {

struct FirstPass : NodeVisitor {
  SymbolTable *_symtab;
  Namespace *_current_ns;

  FirstPass(SymbolTable &symtab);

  const FunctionType &get_function_type(const FunItemBase &item);
  void visit(const FunDeclItem &item) override;
  void visit(const FunDefItem &item) override;
};

struct TypeVisitor : NodeVisitor {
  SymbolTable *_symtab;
  Namespace *_current_ns;
  const Type *type;

  TypeVisitor(SymbolTable &symtab, Namespace &current_ns)
    : _symtab{&symtab}
    , _current_ns{&current_ns}
    , type{nullptr}
  {}

  void visit(const IdentExpr &ident) override;
  void visit(const BinaryExpr &binary) override;
};

} // namespace lava::lang

#endif /* LAVA_LANG_FIRSTPASS_H_ */
