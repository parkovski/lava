#ifndef LAVA_SYN_PARSER_IR_H_
#define LAVA_SYN_PARSER_IR_H_

#include "parser-base.h"
//#include "lava/ir/ir.h"

namespace lava::syn {

struct IRParser : ParserBase {
  using ParserBase::ParserBase;

  void operator()();

private:
  bool try_parse_top();
  void parse_scoped_name();
  void parse_var();
  void parse_fun();
  void parse_fun_args();
  bool try_parse_instr();
  void parse_literal();
};

} // namespace lava::syn

#endif // LAVA_SYN_PARSER_IR_H_
