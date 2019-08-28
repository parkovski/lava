#include "ash/ash.h"
#include "ash/terminal.h"
#include "ash/terminal/ansi.h"
#include "ash/terminal/lineeditor.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <filesystem>

using namespace ash;
using term::LineEditor;

int main() {
  term::initialize();
  term::setShellState();

  ASH_SCOPEEXIT {
    term::restoreState();
  };

  LineEditor ln;
  while (true) {
    auto prompt = fmt::format("{}{}{} ash!{} ",
                term::ansi::fg::bright_blue, std::filesystem::current_path(),
                term::ansi::fg::bright_black, term::ansi::fg::default_);

    switch (ln.readLine(prompt)) {
      using Status = LineEditor::Status;

      case Status::Accepted:
        fmt::print("\n");
        ln.clear();
        break;

      case Status::Canceled:
        fmt::print("{}^C{}\n", term::ansi::fg::red, term::ansi::style::clear);
        ln.clear();
        break;

      case Status::Continue:
      case Status::RedrawPrompt:
        continue;

      case Status::Finished:
        fmt::print("{}%{}\n", term::ansi::style::negative,
                   term::ansi::style::clear);
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

  return 0;
}
