#ifndef LAVA_LANG_IREMIT_H_
#define LAVA_LANG_IREMIT_H_

#include "visitor.h"

namespace lava::lang {

struct IREmitter : NodeVisitor {
  void visit(const ExprItem &item) override;
  void visit(const VarDeclItem &item) override;
  void visit(const FunDeclItem &item) override;
  void visit(const FunDefItem &item) override;
  void visit(const StructDefItem &item) override;
};

} // namespace lava::lang

#endif /* LAVA_LANG_IREMIT_H_ */
