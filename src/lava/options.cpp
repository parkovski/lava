#include "options.h"
#include "cliparser.h"
#include <fmt/format.h>

using namespace std::string_view_literals;

namespace lava::cli {

struct OptionsParser : CliParser {
  explicit OptionsParser(Options &opts, int argc, const char *const *argv)
  noexcept
    : CliParser(argc, argv)
    , _opts{&opts}
  {}

protected:
  int apply_short(char arg, bool more, int &argi) noexcept override;
  int apply_long(std::string_view arg, std::string_view value)
    noexcept override;
  int apply_other(std::string_view arg) noexcept override;

private:
  int invalid_option() noexcept;
  int duplicate_option() noexcept;
  int expected_option(std::string_view what) noexcept;
  int expected_value(std::string_view option,
                     std::string_view values) noexcept;

  Options *_opts;
};

int OptionsParser::apply_short(char arg, bool more, int &argi)
noexcept {
  switch (arg) {
  default:
    return invalid_option();

  case '-':
    if (argi != 0) {
      return invalid_option();
    }
    if (_opts->wants_stdin) {
      return duplicate_option();
    }

    _opts->wants_stdin = 1;
    break;

  case 'e': {
    if (!_opts->eval_source.empty()) {
      return duplicate_option();
    }

    std::string_view value;
    if (!value_short(value, argi)) {
      return expected_option("expression");
    }

    _opts->eval_source = value;
    break;
  }

  case 'h':
    _opts->wants_help = 1;
    break;
  }

  return 0;
}

int OptionsParser::apply_long(std::string_view arg, std::string_view value)
noexcept {
  if (arg == "help"sv) {
    _opts->wants_help = 1;
    return 0;
  }

  if (arg == "eval"sv) {
    if (!_opts->eval_source.empty()) {
      return duplicate_option();
    }

    if (!value_long(value)) {
      return expected_option("expression"sv);
    }

    _opts->eval_source = value;
    return 0;
  }

  if (arg == "color"sv) {
    if (value == "no"sv) {
      _opts->wants_color = 0;
    } else if (value == "yes"sv) {
      _opts->wants_color = 1;
    } else {
      return expected_value("--color", "'yes' or 'no'");
    }
    return 0;
  }

  return invalid_option();
}

int OptionsParser::apply_other(std::string_view arg) noexcept {
  _opts->sources.emplace_back(arg);
  return 0;
}

int OptionsParser::invalid_option() noexcept {
  fmt::print(stderr, "Invalid option '{}'.", (*this)[argn]);
  return 1;
}

int OptionsParser::duplicate_option() noexcept {
  fmt::print(stderr, "Duplicate option '{}'.", (*this)[argn]);
  return 1;
}

int OptionsParser::expected_option(std::string_view what) noexcept {
  fmt::print(stderr, "Expected {} after '{}'", what, (*this)[argn]);
  return 1;
}

int OptionsParser::expected_value(std::string_view option,
                                  std::string_view values) noexcept {
  fmt::print(stderr, "Expected {} after '{}'", values, option);
  return 1;
}

std::variant<Options, int>
Options::from_args(int argc, const char *const *argv) noexcept {
  Options opts;
  auto err = OptionsParser{opts, argc, argv}();
  if (err) {
    return err;
  }
  return opts;
}

} // namespace lava::cli
