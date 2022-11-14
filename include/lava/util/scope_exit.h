#ifndef LAVA_UTIL_SCOPE_EXIT_H_
#define LAVA_UTIL_SCOPE_EXIT_H_

#include "lava/lava.h"
#include <type_traits>
#include <utility>

namespace lava::detail {

  /// RAII wrapper to run a function when the scope exits.
  template<class F, class = std::enable_if_t<std::is_invocable_v<F>>>
  class ScopeExit {
    F _function;
  public:
    explicit ScopeExit(F &&function) noexcept
      : _function(std::move(function))
    {}

    ScopeExit(const ScopeExit &) = delete;
    ScopeExit &operator=(const ScopeExit &) = delete;

    ~ScopeExit() noexcept(std::is_nothrow_invocable_v<F>) {
      _function();
    }
  };

  /// Helper struct to enable the scope exit macros.
  struct ScopeExitHelper{};

  /// Helper operator for scope exit macros.
  template<class F>
  inline ScopeExit<F> operator&&(const ScopeExitHelper &, F function) {
    return ScopeExit<F>{std::move(function)};
  }

} // namespace lava::detail

/// Scope destructor.
/// Usage: LAVA_SCOPE_EXIT { capture-by-ref lambda body };
#define LAVA_SCOPE_EXIT \
  [[maybe_unused]] auto const &LAVA_CONCAT2(_scope_exit_, __LINE__) = \
    ::lava::detail::ScopeExitHelper{} && [&]()

/// Scope destructor that captures the this pointer by value.
/// Usage: LAVA_SCOPE_EXIT_THIS { lambda body using this pointer };
#define LAVA_SCOPE_EXIT_THIS \
  [[maybe_unused]] auto const &LAVA_CONCAT2(_scope_exit_, __LINE__) = \
    ::lava::detail::ScopeExitHelper{} && [&, this]()

#endif // LAVA_UTIL_SCOPE_EXIT_H_
