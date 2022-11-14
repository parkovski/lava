#ifndef LAVA_OPTIONS_H_
#define LAVA_OPTIONS_H_

#include <vector>
#include <string>
#include <filesystem>
#include <variant>

namespace lava::cli {

// Option bool (actually 4-value). The values are picked specifically to fit
// into 2 bits and sign extend correctly.
enum OptBool : signed {
  OptFalse = 0,
  OptTrue  = 1,
  OptAuto  = -2,
  OptNull  = -1,
};

// Specifies where execution should start.
enum StartupMode : unsigned {
  // Read and evaluate from stdin. If the stream does not end, prompt the user
  // for more input. This is the default mode.
  StartupModeInteractive,

  // Start executing from the first source file in argv. This is the default
  // when one or more file names are given.
  StartupModeMainFile,

  // Evaluate an expression passed on the command line.
  StartupModeCliEval,

  // Run the builtin text editor.
  StartupModeEditText,

  // Number of StartupMode entries.
  _StartupMode_count_,
};

static_assert(_StartupMode_count_ == 4,
              "Update startup_mode bitfield in Options");

enum class CliOpt : unsigned {
  Help,
  Stdin,
  Color,
  Eval,
  Interactive,
  Edit,

  _count_,
};

// Driver program options.
struct Options {
  static std::variant<Options, int>
  from_args(int argc, const char *const *argv) noexcept;

  // Source files passed on the command line.
  std::vector<std::filesystem::path> sources;

  // Source passed to `-e` or `--eval`.
  std::string eval_source;

  uint32_t    wants_help   : 1; // Did we see `-h`?
  uint32_t    wants_stdin  : 1; // Did we see `-`?
  OptBool     wants_color  : 2; // `true`/`false`/`auto`.
  StartupMode startup_mode : 2; // Where does execution start?

private:
  explicit Options() noexcept;
};

} // namespace lava::cli

#endif // LAVA_OPTIONS_H_
