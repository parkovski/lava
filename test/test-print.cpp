#include "lava/syn/lexer.h"
#include "lava/syn/parser.h"
#include "lava/syn/parser-ir.h"
#include "lava/syn/printer.h"
#include <fmt/format.h>

using namespace lava;
using namespace std::string_view_literals;

static void print_lex(syn::Lexer &lexer, src::SourceFile &file) {
  syn::SimpleToken token;
  do {
    token = lexer();
    auto loc = lexer.location();

    if (token.span().start().line == loc.line) {
      if (token.span().start().column + 1 == loc.column) {
        fmt::print("{:03}:{:02}        :: {}",
                   token.span().start().line, token.span().start().column,
                   token.id());
      } else {
        fmt::print("{:03}:{:02}-{:02}     :: {}",
                   token.span().start().line, token.span().start().column,
                   loc.column, token.id());
      }
    } else {
      fmt::print("{:03}:{:02}-{:03}:{:02} :: {}",
                 token.span().start().line, token.span().start().column,
                 loc.line, loc.column, token.id());
    }

    switch (token.id()) {
    case syn::Tk::HashBang:
    case syn::Tk::CommentLine:
    case syn::Tk::CommentBlockStart:
    case syn::Tk::CommentBlockText:
    case syn::Tk::CommentBlockEnd:
    case syn::Tk::CommentKeyword:
    case syn::Tk::Text:
    case syn::Tk::Ident:
    case syn::Tk::Variable:
    case syn::Tk::Path:
    case syn::Tk::Escape:
    case syn::Tk::IntLit:
    case syn::Tk::HexLit:
      fmt::print(" '{}'\n", std::string_view{
        &file[token.span().start().index],
        token.span().length()
      });
      break;

    default:
      fmt::print("\n");
      break;
    }
  } while (token.id() != syn::Tk::EndOfInput);
}

static void print_parse(syn::Parser &parser, syn::Visitor &printer) {
  auto node = parser();
  if (!node) {
    fmt::print(stderr, "Parser returned null!\n");
    return;
  }
  node->visit(printer);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fmt::print(stderr, "Usage: test-print [mode] [file].");
    return 1;
  }
  std::filesystem::path path{argv[2]};
  src::SourceFile src{path};
  if (argv[1] == "lex"sv) {
    syn::Lexer lexer{src};
    print_lex(lexer, src);
  } else if (argv[1] == "parse"sv) {
    syn::Parser parser{src};
    syn::Printer printer{src};
    print_parse(parser, printer);
  } else if (argv[1] == "lisp"sv) {
    syn::Parser parser{src};
    syn::PrinterLisp printer{src};
    print_parse(parser, printer);
  } else if (argv[1] == "xml"sv) {
    syn::Parser parser{src};
    syn::PrinterXml printer{src};
    print_parse(parser, printer);
  } else if (argv[1] == "ir"sv) {
    syn::IRParser parser{src};
    try {
      parser();
    } catch (syn::ParseError &e) {
      fmt::print(stderr, "{}:{}: error: {}", e.where().line, e.where().column,
                 e.what());
    }
  } else {
    fmt::print(stderr, "Unknown mode '{}'.", argv[1]);
    return 1;
  }
  return 0;
}
