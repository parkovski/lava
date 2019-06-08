#include "ash/ash.h"
#include "ash/terminal.h"
#include "ash/lineeditor.h"
#include "ash/sym/symboltable.h"

#include <fstream>

// symbol = index into list of interned strings
// path = "string" of symbols (basic_string<index_t>).
// a path can have data and attributes.
// an attribute is also a path=>data mapping?

using namespace ash;

#if 0
static Document getInteractiveDocument() {
  Document doc("stdin");
  //...
  return doc;
}

static Document getSourceDocument(std::string_view path) {
  //...
}

static Document getCommandLineDocument(int argc, char *argv[]) {
  //...
}
#endif

#if 0
static int interactiveMain(int argc, char *argv[]) {
  term::initialize();
  term::setShellState();
  ASH_SCOPEEXIT { term::restoreState(); };

  /* Document doc("stdin");
  LineEditor lned(&doc);
  std::string line;
  while (lned.readLine(line)) {
    doc.append(line);
  } */

  return 0;
}

static int sourceMain(int argc, char *argv[]) {
  // try to read the file.
  if (std::ifstream file{argv[1], std::ios::binary | std::ios::ate}) {
    auto size = file.tellg();
    std::string text(size, '\0');
    file.seekg(0);
    if (!file.read(text.data(), size)) {
      return 1;
    }
    Document doc(argv[1], std::move(text));
  } else {
    return 1;
  }
  return 0;
}

static int commandMain(int argc, char *argv[]) {
  Document doc("command");
  for (int i = 2; i < argc; ++i) {
    doc.append(argv[i]);
    doc.append(' ');
  }
  return 0;
}
#endif

int main(int argc, char *argv[]) {
  using namespace ash;

  if (argc == 1) {
    // interactive mode
    // return interactiveMain(argc, argv);
  } else if (argc == 2) {
    // argv[1] is a source file to execute.
    // return sourceMain(argc, argv);
  } else if (argv[1][0] == '-' && argv[1][1] == 'c' && argv[1][2] == '\0' &&
             argc > 2) {
    // treat the command line as a script
    // return commandMain(argc, argv);
  } else {
    return 1;
  }

  sym::SymbolTable symtab;
  // DocumentAttributeSet pas(symtab);
  // BaseLexer lexer;
  // BaseParser parser;
  // IrEmitter emitter;
  // IrInterpreter interpreter;

  // pas
  //   .register<TextSpan>()
  //   .register<>()
  //   .register<>()
  //   .register<>()

  // Document prelude(pas);

  return 0;
}
