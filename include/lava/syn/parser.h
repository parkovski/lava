#ifndef LAVA_SYN_PARSER_H_
#define LAVA_SYN_PARSER_H_

#include "parser-base.h"
#include "syntax.h"
#include <optional>

namespace lava::syn {

struct Parser : ParserBase {
  using ParserBase::ParserBase;

  NodePtr operator()();

private:
  enum {
    EF_PrecMask   = 0x000000ff,
    EF_FlagMask   = 0xffffff00,
    EF_RTL        = 0x00000100,
    EF_NoComma    = 0x00000200,
    EF_NewLine    = 0x00000400,
    EF_Angle      = 0x00000800,
    EF_Delimited  = 0x00001000,
  };

  constexpr static unsigned get_rtl_bit(unsigned flags) noexcept
  { static_assert((1 << 8) == EF_RTL); return (flags >> 8) & 1; }

  std::unique_ptr<List> many(NodePtr (Parser::*fn)());

  NodePtr parse_top();

  Namespace parse_namespace();
  Infix parse_scoped_name();
  Interface parse_interface();
  NodePtr parse_interface_inner();
  NodePtr parse_struct_union();
  NodePtr parse_struct_union_inner();
  Enum parse_enum();
  NodePtr parse_enum_inner();
  NodePtr parse_type(bool decl_only = false);
  NodePtr parse_fun(bool decl_only = false);
  NodePtr parse_var(bool decl_only = false);

  NodePtr parse_expr(unsigned prec_and_flags);
  NodePtr parse_expr_terminal();
  NodePtr parse_expr_bracketed(unsigned flags);
  NodePtr parse_expr_prefix(unsigned prec_and_flags);
  NodePtr parse_expr_left(NodePtr left, unsigned prec_and_flags);
  NodePtr parse_expr_right(NodePtr left, unsigned prec_and_flags);

  static unsigned prec_prefix(Tk tk, unsigned flags = 0);

  static std::pair<unsigned, unsigned>
  prec_infix_postfix(Tk tk, unsigned flags = 0);

  static constexpr Tk matching_delimiter(Tk tk, unsigned flags);

  // NodePtr parse_string();

  std::unique_ptr<Leaf> take_as_leaf()
  { return std::make_unique<Leaf>(take()); }
};

} // namespace lava::syn

#endif // LAVA_SYN_PARSER_H_
