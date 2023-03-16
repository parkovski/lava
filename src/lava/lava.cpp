#include "lava/lava.h"
#include "cliparser.h"
#include "options.h"
#include <fmt/format.h>

using namespace lava;
using namespace lava::cli;

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
  --lsp             Run in language server mode.
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

  if (opts.startup_mode == StartupModeLSPServer) {
    if (opts.wants_stdin) {
      fmt::print(stderr, "LSP mode and read from stdin can't be combined.\n");
      return 1;
    }
    if (!opts.eval_source.empty()) {
      fmt::print(stderr, "LSP mode and CLI eval can't be combined.\n");
      return 1;
    }
    if (!opts.sources.empty()) {
      fmt::print(stderr, "LSP mode can't be provided with source files.\n");
      return 1;
    }
    fmt::print(stderr, "LSP server TODO.\n");
    return 2;
  }

  if (!opts.eval_source.empty()) {
    fmt::print("TODO: Command line eval\n");
  }

  for (auto &source : opts.sources) {
    auto absolute_path = std::filesystem::absolute(source);
    auto path_str = absolute_path.string();
    fmt::print("TODO: Add source {}\n", path_str);
  }

  if (opts.startup_mode == StartupModeInteractive) {
    fmt::print("TODO: Interactive mode\n");
  } else {
    assert(opts.startup_mode == StartupModeBatch);
    fmt::print("TODO: Process files\n");
  }

  return 0;
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
int wmain() {
  int argc;
  wchar_t **argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (!argvw) {
    return GetLastError();
  }

  char **argv = new char*[argc];
  LAVA_SCOPE_EXIT {
    for (int i = 0; i < argc; ++i) {
      if (argv[i]) delete [] argv[i];
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
