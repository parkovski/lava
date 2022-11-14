#include "lava/lava.h"
#include "cliparser.h"
#include "options.h"
#include <fmt/format.h>

using namespace lava::cli;

#if 0
static int sourceMain(const char *filename) {
  if (std::ifstream file{filename, std::ios::binary | std::ios::ate}) {
    auto size = file.tellg();
    std::string text(size, '\0');
    file.seekg(0);
    if (!file.read(text.data(), size)) {
      fmt::print(stderr, "Could not read `{}`.\n", filename);
      return 1;
    }
    return lava::eval(text);
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

  return lava::eval(program);
}
#endif

static void print_help(bool islong) {
  (void)islong;

  fmt::print(
R"===(Usage: lava [options...] [main.lava] [script.lava...] [arguments...]

Options:
  -h, --help        Print this help message.
  -                 Read script from stdin.
  --                Stop parsing Lava options.

  --color=[bool]    Use color when printing diagnostics.
  -e, --eval=...    Evaluate expression.
  -i, --interactive Interactive mode; default if no files are specified and
                    stdin is a tty. Necessary if specifying other scripts to
                    load on the command line.
  -E, --edit        Basic text editor mode.
)==="
  );
}

#ifndef _WIN32
int main(int argc, char **argv)
#else
static int u8main(int argc, char **argv)
#endif
{
  auto opts_or_error = Options::from_args(argc, argv);
  if (std::holds_alternative<int>(opts_or_error)) {
    return std::get<int>(opts_or_error);
  }

  auto opts = std::get<Options>(opts_or_error);

  if (opts.wants_help) {
    print_help(false);
    return 0;
  }

  switch (opts.startup_mode) {
  case StartupModeInteractive:

  case StartupModeCliEval:
    break;

  case StartupModeMainFile:
    break;

  case StartupModeEditText:
    break;

  default:
    LAVA_UNREACHABLE();
  }

  return 0;
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
int wmain() {
  int argc;
  wchar_t *args = GetCommandLineW();
  wchar_t **argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (!argvw) {
    return GetLastError();
  }

  char **argv = new char*[argc];
  LAVA_SCOPE_EXIT {
    for (int i = 0; i < argc; ++i) {
      delete [] argv[i];
    }
    delete [] argv;
  };

  {
    LAVA_SCOPE_EXIT { LocalFree(argvw); };

    for (int i = 0; i < argc; ++i) {
      auto len = WideCharToMultiByte(
        CP_UTF8, 0, argvw[i], -1, nullptr, 0, nullptr, nullptr
      );
      argv[i] = new char[len + 1];
      if (!WideCharToMultiByte(
        CP_UTF8, 0, argvw[i], -1, argv[i], wlen, nullptr, nullptr
      )) {
        return GetLastError();
      } else {
        argv[i][len] = 0;
      }
    }
  }

  return u8main(argc, argv);
}
#endif // _WIN32
