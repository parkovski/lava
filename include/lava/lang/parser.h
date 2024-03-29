#ifndef LAVA_LANG_PARSER_H_
#define LAVA_LANG_PARSER_H_

#include "nodes.h"
#include "lexer.h"
#include <stdexcept>

namespace lava::lang {

struct ParseError : std::runtime_error {
private:
  SourceLoc _loc;

public:
  ParseError(SourceLoc loc, std::string what)
    : runtime_error(std::move(what))
    , _loc{loc}
  {}

  SourceLoc where() const { return _loc; }
};

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
  std::unique_ptr<Document> parse_document();

  std::unique_ptr<Item> parse_item();

  std::unique_ptr<VarDeclItem> parse_var_item(std::unique_ptr<Expr> type);
  std::optional<VarDecl> parse_var_decl();
  std::optional<VarInit> parse_var_init();

  std::unique_ptr<FunItemBase> parse_fun_item();
  std::optional<ArgList> parse_arg_list();
  std::optional<ArgDecl> parse_arg_decl();

  std::unique_ptr<StructDefItem> parse_struct_or_union();

  std::unique_ptr<Expr> parse_expr(int flags = 0, unsigned prec = 1);
  std::optional<ScopeExpr> parse_scope_expr();
  std::unique_ptr<InvokeExpr> parse_invoke_expr(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> parse_primary();
  std::unique_ptr<IfExpr> parse_if();
  std::unique_ptr<WhileExpr> parse_while();
  std::unique_ptr<LoopExpr> parse_loop();

private:
  static unsigned get_prefix_prec(int op, int flags);
  static unsigned get_infix_prec(int op, int flags);
  static unsigned get_postfix_prec(int op, int flags);
  static bool is_rtl_operator(int op);
};

} // namespace lava::lang

#endif // LAVA_LANG_PARSER_H_
