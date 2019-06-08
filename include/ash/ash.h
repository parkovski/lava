#ifndef ASH_ASH_H_
#define ASH_ASH_H_

#define ASH_ARRAYLEN(arr) (sizeof(arr) / sizeof(arr[0]))

#include <type_traits>

namespace ash::detail {

  /// RAII wrapper to run a function when the scope exits.
  template<typename OnExit,
           typename = std::enable_if_t<std::is_invocable_v<OnExit>>>
  class ScopeExit {
    OnExit _onExit;
  public:
    explicit ScopeExit(OnExit &&onExit) noexcept
      : _onExit(std::move(onExit))
    {}

    ScopeExit(const ScopeExit &) = delete;
    ScopeExit &operator=(const ScopeExit &) = delete;

    ~ScopeExit() noexcept(std::is_nothrow_invocable_v<OnExit>) {
      _onExit();
    }
  };

  /// Helper struct to enable the scope exit macros.
  struct ScopeExitHelper{};

  /// Helper operator for scope exit macros.
  template<typename OnExit>
  inline ScopeExit<OnExit> operator^(const ScopeExitHelper &, OnExit onExit) {
    return ScopeExit<OnExit>{std::move(onExit)};
  }

} // namespace ash::detail

#define ASH_CONCAT_IMPL(a,b) a##b
#define ASH_CONCAT2(a,b) ASH_CONCAT_IMPL(a,b)

/// Scope destructor.
/// Usage: ASH_SCOPEEXIT { capture-by-ref lambda body };
#define ASH_SCOPEEXIT \
  [[maybe_unused]] auto const &ASH_CONCAT2(_scopeExit_, __LINE__) = \
    ::ash::detail::ScopeExitHelper{} ^ [&]()

/// Scope destructor that captures the this pointer by value.
/// Usage: ASH_SCOPEEXIT_THIS { lambda body using this pointer };
#define ASH_SCOPEEXIT_THIS \
  [[maybe_unused]] auto const &ASH_CONCAT2(_scopeExit_, __LINE__) = \
    ::ash::detail::ScopeExitHelper{} ^ [&, this]()

#endif /* ASH_ASH_H_ */

