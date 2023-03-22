#include "lava/driver/options.h"
#include "lava/driver/cliparser.h"
#include "lava/term/terminal.h"
#include <fmt/format.h>

#include <boost/algorithm/string.hpp>

using namespace std::string_view_literals;

namespace lava::driver {

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
  int apply_option(CliOpt opt, std::string_view value) noexcept;

  int invalid_option() noexcept;
  int invalid_option(std::string_view reason) noexcept;
  int duplicate_option() noexcept;
  int expected_option(std::string_view what) noexcept;
  int expected_value(std::string_view option,
                     std::string_view values) noexcept;

  /// @param value Boolean-ish CLI option value.
  /// @param allow_auto Allow the value "auto" (or unset).
  /// @return For truthy values (yes, on, true, 1), returns `OptTrue`.
  ///         For falsey values (no, off, false, 0), returns `OptFalse`.
  ///         If `allow_auto` is true, returns `OptAuto` for `auto` or empty.
  ///         Otherwise returns `OptNull` for invalid input.
  OptBool get_bool_value(std::string_view value, bool allow_auto = false)
                        noexcept;

  Options *_opts;
};

int OptionsParser::apply_short(char arg, bool more, int &argi)
                              noexcept {
  CliOpt opt;
  std::string_view value{};
  switch (arg) {
  default:
    return invalid_option();

  case '-':
    opt = CliOpt::Stdin;
    if (argi != 0) {
      // Dash option must be in position 0 - not combined with other options.
      return invalid_option();
    }
    break;

  case 'e':
    opt = CliOpt::Eval;
    if (!value_short(value, argi)) {
      return expected_option("expression");
    }
    break;

  case 'h':
    opt = CliOpt::Help;
    break;

  case 'i':
    opt = CliOpt::Interactive;
    break;
  }

  return apply_option(opt, value);
}

int OptionsParser::apply_long(std::string_view arg, std::string_view value)
noexcept {
  CliOpt opt;

  if (arg == "help"sv) {
    opt = CliOpt::Help;
  } else if (arg == "color"sv) {
    opt = CliOpt::Color;
    if (!value_long(value)) {
      return expected_value("--color", "yes/no/auto");
    }
  } else if (arg == "eval"sv) {
    opt = CliOpt::Eval;
    if (!value_long(value)) {
      return expected_option("expression"sv);
    }
  } else if (arg == "interactive"sv) {
    opt = CliOpt::Interactive;
  } else if (arg == "lsp"sv) {
    opt = CliOpt::LSPServer;
  } else {
    return invalid_option();
  }

  return apply_option(opt, value);
}

int OptionsParser::apply_other(std::string_view arg) noexcept {
  _opts->sources.emplace_back(arg);
  return 0;
}

int OptionsParser::apply_option(CliOpt opt, std::string_view value) noexcept {
  switch (opt) {
  default:
    return invalid_option();

  case CliOpt::Help:
    _opts->wants_help = 1;
    break;

  case CliOpt::Stdin:
    if (_opts->wants_stdin) {
      return duplicate_option();
    }
    _opts->wants_stdin = 1;
    break;

  case CliOpt::Color:
    if ((_opts->wants_color = get_bool_value(value)) == OptNull) {
      return expected_option("yes/no/auto");
    }
    break;

  case CliOpt::Eval:
    if (!_opts->eval_source.empty()) {
      // Add another source line for each eval option.
      _opts->eval_source.append(1, '\n');
    }
    _opts->eval_source.append(value);
    break;

  case CliOpt::Interactive:
    if (_opts->startup_mode != StartupModeAutomatic) {
      return invalid_option("Ambiguous/duplicate startup mode");
    }
    _opts->startup_mode = StartupModeInteractive;
    break;

  case CliOpt::LSPServer:
    if (_opts->startup_mode != StartupModeAutomatic) {
      return invalid_option("Ambiguous/duplicate startup mode");
    }
    _opts->startup_mode = StartupModeLSPServer;
    break;
  }

  return 0;
}

int OptionsParser::invalid_option() noexcept {
  fmt::print(stderr, "Invalid option '{}'.", (*this)[argn]);
  return 1;
}

int OptionsParser::invalid_option(std::string_view reason) noexcept {
  fmt::print(stderr, "Invalid option '{}': {}.", (*this)[argn], reason);
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

OptBool OptionsParser::get_bool_value(std::string_view value, bool allow_auto)
                                     noexcept {
  if (boost::algorithm::iequals(value, "on"sv)
   || boost::algorithm::iequals(value, "yes"sv)
   || boost::algorithm::iequals(value, "true"sv)
   || value == "1"sv) {
    return OptTrue;
  }

  if (boost::algorithm::iequals(value, "off"sv)
   || boost::algorithm::iequals(value, "no"sv)
   || boost::algorithm::iequals(value, "false"sv)
   || value == "0"sv) {
    return OptFalse;
  }

  if (allow_auto && (value.empty()
                  || boost::algorithm::iequals(value, "auto"sv))) {
    return OptAuto;
  }

  return OptNull;
}

Options::Options() noexcept
  : wants_help{0}
  , wants_stdin{0}
  , wants_color{OptAuto}
  , startup_mode{StartupModeAutomatic}
{}

std::variant<Options, int>
Options::from_args(int argc, const char *const *argv) noexcept {
  Options opts;
  auto err = OptionsParser{opts, argc, argv}();
  if (err) {
    return err;
  }
  if (opts.startup_mode == StartupModeAutomatic) {
    if (opts.wants_stdin) {
      opts.startup_mode = term::isTTYInput()
        ? StartupModeInteractive : StartupModeBatch;
    } else if (!opts.eval_source.empty() || !opts.sources.empty()) {
      opts.startup_mode = StartupModeBatch;
    } else {
      opts.startup_mode = StartupModeInteractive;
    }
  }
  return opts;
}

} // namespace lava::driver
