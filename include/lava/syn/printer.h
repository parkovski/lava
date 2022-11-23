#ifndef LAVA_SYN_PRINTER_H_
#define LAVA_SYN_PRINTER_H_

#include "syntax.h"
#include "../src/source.h"

#define LAVA_DECLARE_VISITOR \
  void visit_leaf(Leaf &node) override final;           \
  void visit_adjacent(Adjacent &node) override final;   \
  void visit_bracketed(Bracketed &node) override final; \
  void visit_unary(Unary &node) override final;         \
  void visit_infix(Infix &node) override final

namespace lava::syn {

struct Printer : Visitor {
  Printer(src::SourceFile &src)
    : _src{&src}
  {}

  LAVA_DECLARE_VISITOR;

  void visit_trivia(const Token::TriviaList &trivia);

  src::SourceFile *_src;
};

struct PrinterLisp : Visitor {
  PrinterLisp(src::SourceFile &src)
    : _src{&src}
  {}

  LAVA_DECLARE_VISITOR;

  src::SourceFile *_src;
};

struct PrinterXml : Visitor {
  PrinterXml(src::SourceFile &src);

  LAVA_DECLARE_VISITOR;

  src::SourceFile *_src;
};

} // namespace lava::syn

#endif /* LAVA_SYN_PRINTER_H_ */
