#include "lava/syn/syntax.h"
#include "lava/syn/printer.h"
#include <fmt/format.h>

using namespace lava;
using namespace std::string_view_literals;

static const char *text =
R"(include "lava/syn/syntax.h"
using namespace lava;
fun reconstruct() -> syn.NodePtr {
  return read $this_file |> Parser.parse;
})";

static syn::NodePtr reconstruct() {
  return nullptr;
}

int main(int argc, char *argv[]) {
  Printer p;
}
