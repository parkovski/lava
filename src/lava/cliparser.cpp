#include "cliparser.h"
#include <cassert>
#include <limits>

using namespace lava::cli;

int CliParser::operator()() noexcept {
  this->argn = 1;
  for (; this->argn < _argc; ++this->argn) {
    const char *arg = _argv[this->argn];
    if (arg[0] == '\0') {
      // Skip empty arguments.
      continue;
    }

    if (arg[0] == '-') {
      if (arg[1] == '\0') {
        // Argument is a single `-`.
        int argi = 0;
        auto ret = this->apply_short('-', false, argi);
        if (ret != 0) {
          return ret;
        }
      } else if (arg[1] == '-') {
        if (arg[2] == '\0') {
          // Stop parsing. Argument is `--`.
          ++this->argn;
          break;
        } else {
          // Long option `--arg`.
          // At this point, `arg` is known to be at least 3 bytes and arg[2]
          // must be part of the argument name. Start looking for the `=` at
          // arg[3].
          const char *eq = nullptr, *end = &arg[3];
          for (; *end; ++end) {
            if (*end == '=' && !eq) {
              eq = end;
            }
          }

          std::string_view name, value;
          if (eq) {
            name = std::string_view{
              arg + 2,
              static_cast<size_t>(eq - arg - 2)
            };
            value = std::string_view{
              eq + 1,
              static_cast<size_t>(end - eq - 1)
            };
          } else {
            name = std::string_view{
              arg + 2,
              static_cast<size_t>(end - arg - 2)
            };
            value = std::string_view{};
          }

          if (auto ret = this->apply_long(name, value); ret != 0) {
            return ret;
          }
        }
      } else {
        // Short option `-a` or list of short options `-abc`.
        int prev_argn = argn;
        int argi = 1;
        int len = static_cast<int>(std::char_traits<char>::length(arg));
        for (; argi < len - 1; ++argi) {
          auto ret = this->apply_short(arg[argi], true, argi);
          if (ret) {
            return ret;
          }
          if (prev_argn != argn) {
            // User changed `argn`; don't continue with this arg.
            break;
          }
        }
        if (prev_argn == argn && argi == len - 1) {
          // Test final character only if user didn't modify `argn` or `argi`.
          auto ret = this->apply_short(arg[argi], false, argi);
          if (ret != 0) {
            return ret;
          }
        }
      }
    }
#ifdef _WIN32
    else if (arg[0] == '/') {
      if (arg[1] == '\0') {
        // Single `/`. I guess this is a short option but it probably should be
        // an error.
        int argi = 0;
        if (auto ret = this->apply_short('/', false, argi); ret != 0) {
          return ret;
        }
      } else if (arg[2] == '\0') {
        // Single character `/a`.
        int argi = 1;
        if (auto ret = this->apply_short(arg[1], false, argi); ret != 0) {
          return ret;
        }
      } else if (arg[2] == ':') {
        // Short option `/a:Value`.
        int argi = 2; // Start value after the `:`.
        auto ret = this->apply_short(arg[1], true, argi);
        assert(argi > 2 && "Argument value not consumed");
        if (ret) {
          return ret;
        }
      } else {
        // Long option: '/arg' or '/arg:Value'.
        const char *colon = nullptr, *end = &arg[1];
        for (; *end; ++end) {
          if (*end == ':' && !colon) {
            colon = end;
          }
        }

        std::string_view name, value;
        if (colon) {
          name = std::string_view{
            arg + 1,
            static_cast<size_t>(colon - arg - 1)
          };
          value = std::string_view{
            colon + 1,
            static_cast<size_t>(end - colon - 1)
          };
        } else {
          name = std::string_view{arg + 1, static_cast<size_t>(end - arg - 1)};
          value = std::string_view{};
        }

        if (auto ret = this->apply_long(name, value, i); ret != 0) {
          return ret;
        }
      }
    }
#endif
    else {
      if (auto ret = this->apply_other(std::string_view{arg}); ret != 0) {
        return ret;
      }
    }
  }

  // If a stop parsing '--' was encountered; pass the rest of the args as
  // 'other'.
  for (; this->argn < _argc; ++this->argn) {
    std::string_view arg{_argv[this->argn]};
    if (auto ret = this->apply_other(arg); ret != 0) {
      return ret;
    }
  }

  return 0;
}

bool CliParser::value_short(std::string_view &value, int &argi)
noexcept {
  if (auto arg = (*this)[argn]; arg[argi + 1]) {
   // Current arg contains more characters; consume them.
    value = &arg[argi + 1];
    // Set to INT_MAX - 1 so that when the loop increments this it won't
    // overflow.
    argi = std::numeric_limits<int>::max() - 1;
    return true;
  }

  if (argn + 1 < _argc) {
    // Nothing left in this argument; take the next one.
    value = (*this)[++argn];
    return true;
  }

  // No arguments are left, couldn't get the value.
  return false;
}

bool CliParser::value_long(std::string_view &value) noexcept {
  if (!value.empty()) {
    return true;
  }

  if (argn + 1 < _argc) {
    // Value is empty, take next argument.
    value = (*this)[++argn];
    return true;
  }

  // Value is empty, but no arguments are left.
  return false;
}
