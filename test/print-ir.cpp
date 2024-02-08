#include <fstream>
#include <optional>
#include <fmt/format.h>
#include "lava/lang/parser.h"
#include "lava/lang/firstpass.h"
#include "lava/lang/iremit.h"

using namespace lava::lang;

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
  SourceDoc doc{
    argv[1], std::move(content).value()
  };

  Lexer lexer{doc};
  Parser parser{lexer};

  auto document = parser.parse_document();

  PointerType::TargetPointerSize = sizeof(size_t);
  SymbolTable symtab;
  FirstPass fp{symtab};
  fp.NodeVisitor::visit(*document);
  IREmitter ire{symtab};
  ire.NodeVisitor::visit(*document);

  for (size_t i = 0; i < symtab.global_namespace().size(); ++i) {
    auto sym = symtab.global_namespace()[i];
    if (auto fn = dynamic_cast<Function*>(sym)) {
      fmt::print("function {}:\n", symtab.get_string(fn->name()));
      for (size_t j = 0; j < fn->basicblocks().size(); ++j) {
        fmt::print("#{}:\n", j);
        auto const &bb = fn->basicblocks()[j];
        for (size_t k = 0; k < bb.instrs.size(); ++k) {
          fmt::print("{}\n", instr_to_string(bb.instrs[k]));
        }
      }
    }
  }
}
