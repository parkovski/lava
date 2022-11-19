#ifndef LAVA_SYN_PRINTER_H_
#define LAVA_SYN_PRINTER_H_

#include "syntax.h"
#include "../src/source.h"

namespace lava::syn {

struct Printer : Visitor {
  Printer(src::SourceFile &src)
    : _src{&src}
  {}

  void visit_trivia(const TriviaList &trivia);
  void visit_leaf(Leaf &node) override final;
  void visit_adjacent(Adjacent &node) override final;
  void visit_bracketed(Bracketed &node) override final;
  void visit_unary(Unary &node) override final;
  void visit_infix(Infix &node) override final;

  src::SourceFile *_src;
};

} // namespace lava::syn

#endif /* LAVA_SYN_PRINTER_H_ */
