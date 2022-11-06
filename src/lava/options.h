#ifndef LAVA_OPTIONS_H_
#define LAVA_OPTIONS_H_

#include <vector>
#include <string>
#include <filesystem>
#include <variant>

namespace lava::cli {

// Driver program options.
struct Options {
  static std::variant<Options, int>
  from_args(int argc, const char *const *argv) noexcept;

  // Source files passed on the command line.
  std::vector<std::filesystem::path> sources;

  // Source passed to `-e` or `--eval`.
  std::string eval_source;

  uint32_t wants_help : 1;
  uint32_t wants_stdin : 1;
  uint32_t wants_color : 1;

private:
  Options() = default;
};

} // namespace lava::cli

#endif // LAVA_OPTIONS_H_
