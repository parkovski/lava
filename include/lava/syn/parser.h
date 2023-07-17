#ifndef LAVA_SYN_PARSER_H_
#define LAVA_SYN_PARSER_H_

#include "lexer.h"
#include "nodes.h"
#include <memory>
#include <vector>
#include <optional>

namespace lava::syn {

struct Parser {
private:
  Lexer *lexer;
  Token token;

public:
  enum Flags {
    PF_NoComma = 1
  };

  explicit Parser(Lexer &lexer) noexcept;

private:
  void next();
  Token take();

public:
  std::unique_ptr<Item> parse_item();

  std::unique_ptr<VarDeclItem> parse_var_item();
  std::optional<VarDecl> parse_var_decl();
  std::optional<VarInit> parse_var_init();

  std::unique_ptr<FunItemBase> parse_fun_item();
  std::optional<ArgList> parse_arg_list();
  std::optional<ArgDecl> parse_arg_decl();
  std::optional<ScopeExpr> parse_fun_body();

  std::unique_ptr<Expr> parse_expr(int flags = 0, unsigned prec = 1);

  std::unique_ptr<InvokeExpr> parse_invoke_expr(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> parse_primary();

private:
  static unsigned get_prefix_prec(int op, int flags);
  static unsigned get_infix_prec(int op, int flags);
  static unsigned get_postfix_prec(int op, int flags);
};

} // namespace lava::syn

#endif // LAVA_SYN_PARSER_H_
