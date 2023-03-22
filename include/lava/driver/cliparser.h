#ifndef LAVA_DRIVER_CLIPARSER_H_
#define LAVA_DRIVER_CLIPARSER_H_

#include <string_view>

namespace lava::driver {

struct CliParser {
  /// Construct a `CliParser` from `argc` and `argv`.
  /// @param argc Size of argument vector.
  /// @param argv Argument vector.
  explicit CliParser(int argc, const char *const *argv) noexcept
    : _argc{argc}
    , _argv{argv}
  {};

  /// Parse the command line arguments.
  /// @return `0` if successful, otherwise an exit code.
  int operator()() noexcept;

protected:
  /// @return Number of arguments.
  int argc() const noexcept
  { return _argc; }

  /// @return Argument vector.
  const char *const *argv() const noexcept
  { return _argv; }

  /// Bounds checked argument index.
  /// @param i Argument index.
  /// @return Argument number `i` or null if `i` is out of range.
  const char *argv(int n) const noexcept
  { return n < _argc ? _argv[n] : nullptr; }

  /// Unchecked argument index.
  /// @param i Argument index.
  /// @return Argument number `i`.
  const char *operator[](int n) const noexcept
  { return _argv[n]; }

  /// Get the value of a short option, either '-a Value' or '-aValue' (on
  /// Windows, also '/a:Value' or '/a Value') and update `argn` and `argi`.
  /// @param value [out] The option value.
  /// @param argi [inout] Character index in current argument.
  /// @return `true` if the value could be retrieved, otherwise `false`.
  bool value_short(std::string_view &value, int &argi) noexcept;

  /// Get the value of a long option, either '--arg=Value' or '--arg Value' (on
  /// Windows, also '/arg:Value' or '/arg Value').
  /// @param value [inout] The option value passed to `apply_long`. If unset,
  ///        it will be taken from the next argument.
  /// @param argn [inout] Current index in `argv`.
  /// @return `true` if the value could be retrieved, otherwise `false`.
  bool value_long(std::string_view &value) noexcept;

  /// Handle a single character (`-a`) argument. Multiple arguments can be
  /// listed after the `-`. On Windows, also matches a single argument in the
  /// form `/a`.
  /// @param c Argument character.
  /// @param more `true` if there are more arguments in this set, otherwise
  ///        false.
  /// @param [inout] argi Character index in current argument.
  /// @return `0` if the argument is valid, otherwise the program exit code.
  virtual int apply_short(char arg, bool more, int &argi) noexcept
    = 0;

  /// Handle a long (`--arg{=value}`) argument. On Windows, also matches an
  /// argument in the form `/arg:value`.
  /// @param arg Argument name.
  /// @param value Argument value if applicable, or empty.
  /// @return `0` if the argument is valid, otherwise the program exit code.
  virtual int apply_long(std::string_view arg,
                         std::string_view value) noexcept = 0;

  /// Handle a non-flag (not starting with `-` or on Windows, `/`) argument or
  /// any argument after `--`.
  /// @param arg Current argument.
  /// @return `0` if the argument is valid, otherwise the program exit code.
  virtual int apply_other(std::string_view arg) noexcept = 0;

  // Current index in `_argv`.
  int argn;

private:
  int _argc;
  const char *const *_argv;
};

} // namespace lava::driver

#endif // LAVA_DRIVER_CLIPARSER_H_
