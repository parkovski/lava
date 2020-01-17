#include "ash/ash.h"
#include "ash/terminal.h"
#include "ash/terminal/ansi.h"
#include "ash/terminal/lineeditor.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <filesystem>
#include <cstdlib>

using namespace ash;
using term::LineEditor;

size_t nextPathSep(const std::string &s, size_t start) {
  size_t slash = s.find('/', start);
#if _WIN32
  size_t backslash = s.find('\\', start);
  // Ok even if slash or backslash is npos as it is the greatest size_t value.
  if (backslash < slash) {
    slash = backslash;
  }
#endif
  return slash;
}

void abbreviatePath(std::string &path) {
  size_t sep;
#ifdef _WIN32
  if (path.length() > 2 && path[1] == ':' &&
           (path[2] == '\\' || path[2] == '/')) {
    // Drive path: C:\...
    sep = 2;
  } else if (path.length() > 2 && path[0] == '\\' && path[1] == '\\') {
    // UNC path: \\...
    sep = 1;
  }
#else
  if (path.length() && path[0] == '/') {
    // Unix absolute path: /...
    sep = 0;
  }
#endif
  else {
    // Regular path part.
    sep = nextPathSep(path, 0);
  }
  size_t nextSep;
  while ((nextSep = nextPathSep(path, sep + 1)) != std::string::npos) {
    if (nextSep == sep + 1) {
      // Duplicate separator, delete it.
      path.erase(nextSep);
    } else if (nextSep > sep + 2) {
      // More than one character, shorten path part.
      sep += 2;
      path.erase(sep, nextSep - sep);
    } else {
      sep = nextSep;
    }
  }
}

int readLoop() {
  LineEditor ln;
  std::string_view home{};
  if (auto homeEnv = getenv("HOME"); homeEnv && *homeEnv) {
    home = homeEnv;
    fmt::print("Home is \"{}\"\n", home);
  }
  std::string cwd = std::filesystem::current_path();
  if (home.length() && (std::string_view(cwd).substr(0, home.length()) == home)) {
    cwd.erase(0, home.length() - 1);
    cwd[0] = '~';
  }
  abbreviatePath(cwd);
  auto prompt = fmt::format("{}{}{} ash!{} ",
              term::ansi::fg::bright_blue, cwd,
              term::ansi::fg::bright_black, term::ansi::fg::default_);

  while (true) {
    switch (ln.readLine(prompt)) {
      using Status = LineEditor::Status;

      case Status::Accepted:
        fmt::print("\n{}\n", ln.substr(0));
        ln.clear();
        break;

      case Status::Canceled:
        fmt::print("{}^C{}\n", term::ansi::fg::red, term::ansi::style::clear);
        ln.clear();
        break;

      case Status::Finished:
        fmt::print("\n");
        return 0;

      case Status::ReadError:
        fmt::print(stderr, "{}Couldn't read from stdin.{}",
                   term::ansi::bg::red + term::ansi::fg::white,
                   term::ansi::style::clear);
        return 1;

      default:
        fmt::print(stderr, "{}LineEditor::readLine{} returned unknown value.{}",
                   term::ansi::bg::red + term::ansi::fg::bright_yellow,
                   term::ansi::fg::white, term::ansi::style::clear);
        return 2;
    }
  }
}

int main() {
  term::initialize();
  term::setShellState();

  ASH_SCOPEEXIT {
    term::restoreState();
  };

  return readLoop();
}
