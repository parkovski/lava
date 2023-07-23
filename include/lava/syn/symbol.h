#ifndef LAVA_SYN_SYMBOL_H_
#define LAVA_SYN_SYMBOL_H_

#include <string>
#include <string_view>

namespace lava::syn {

enum class SymbolKind {
  Scope,
  Type,
  Variable,
  Function,
};

struct Symbol {
private:
  std::string _name;

public:
  explicit Symbol(std::string name) noexcept
    : _name{std::move(name)}
  {}

  virtual ~Symbol() = 0;

  std::string_view name() const { return _name; }
  virtual SymbolKind symbol_kind() const = 0;
};

} // namespace lava::syn

#endif // LAVA_SYN_SYMBOL_H_
