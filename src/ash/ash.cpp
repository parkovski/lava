#include "ash/ash.h"

#include <fmt/format.h>

#include <string_view>
#include <fstream>

namespace ash {
  int eval(std::string_view code);
  int interactiveMain();
}

static int sourceMain(const char *filename) {
  if (std::ifstream file{filename, std::ios::binary | std::ios::ate}) {
    auto size = file.tellg();
    std::string text(size, '\0');
    file.seekg(0);
    if (!file.read(text.data(), size)) {
      fmt::print(stderr, "Could not read `{}`.\n", filename);
      return 1;
    }
    return ash::eval(text);
  }

  fmt::print(stderr, "Could not open `{}`.\n", filename);
  return 1;
}

static int commandMain(int argc, const char *const argv[]) {
  if (argc == 0) {
    fmt::print(stderr, "Nothing to evaluate\n");
    return 1;
  }

  std::string program{argv[0]};
  for (int i = 1; i < argc; ++i) {
    program.push_back(' ');
    program.append(argv[i]);
  }

  return ash::eval(program);
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    return ash::interactiveMain();
  } else if (argc == 2) {
    // argv[1] is a source file to execute.
    return sourceMain(argv[1]);
  } else if (argv[1][0] == '-' && argv[1][1] == 'e' && argv[1][2] == '\0' &&
             argc > 2) {
    // treat the command line as a script
    return commandMain(argc - 2, &argv[2]);
  } else {
    fmt::print(stderr, "Bad command line.\n");
    return 1;
  }
}
