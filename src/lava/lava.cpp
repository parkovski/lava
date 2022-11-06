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

#ifndef _WIN32
int main(int argc, char **argv)
#else
static int win32_main(int argc, char **argv)
#endif
{
  auto opts_or_error = Options::from_args(argc, argv);
  if (std::holds_alternative<int>(opts_or_error)) {
    return std::get<int>(opts_or_error);
  }

  auto opts = std::get<Options>(opts_or_error);

  fmt::print("help? {}\nstdin? {}\ncolor? {}\n",
             (bool)opts.wants_help,
             (bool)opts.wants_stdin,
             (bool)opts.wants_color);
  for (auto const &src : opts.sources) {
    fmt::print("> {}\n", src.native());
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

  return win32_main(argc, argv);
}
#endif // _WIN32
