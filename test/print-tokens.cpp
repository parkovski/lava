#include <fmt/format.h>
#include <fstream>
#include <optional>
#include "lava/lang/lexer.h"

std::optional<std::string> read_file(const char *filename) {
  constexpr size_t buf_size = 4096;
  std::ifstream ifs{filename, std::ios::in | std::ios::binary};

  if (!ifs) {
    return std::nullopt;
  }

  std::string content;
  char buf[buf_size];
  while (ifs.read(buf, buf_size)) {
    content.append(buf, ifs.gcount());
  }
  content.append(buf, ifs.gcount());
  return content;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fmt::print(stderr, "Expected filename.\n");
    return 1;
  }
  auto content = read_file(argv[1]);
  if (!content.has_value()) {
    fmt::print(stderr, "Open file error.\n");
    return 1;
  }
  lava::lang::SourceDoc doc{
    argv[1], std::move(content).value()
  };

  lava::lang::Lexer lexer{doc};
  while (true) {
    auto token = lexer.lex();
    fmt::print("{}:{}:{}: {}\n", token.start.line, token.start.column,
               lava::lang::get_token_name(token.what), token.text());
    if (token.what == lava::lang::TkEof) {
      break;
    }
  }

  return 0;
}
