#ifndef LAVA_SYN_SYMBOL_H_
#define LAVA_SYN_SYMBOL_H_

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <unordered_map>

namespace lava::syn {

enum class SymbolKind {
  Namespace,
  Scope,
  Type,
  Global,
  Local,
  Field,
  Function,
};

struct Symbol {
private:
  std::string _name;

public:
  explicit Symbol() noexcept
    : _name{}
  {}

  explicit Symbol(std::string name) noexcept
    : _name{std::move(name)}
  {}

  virtual ~Symbol() = 0;

  const std::string &name() const { return _name; }
  virtual SymbolKind symbol_kind() const = 0;
};

struct Namespace final : Symbol {
private:
  Namespace *_parent;
  std::vector<std::unique_ptr<Symbol>> _symbols;
  std::unordered_map<std::string_view, size_t> _symbol_names;

public:
  // Global namespace constructor
  explicit Namespace() noexcept
    : _parent{nullptr}
  {}

  // Anonymous namespace constructor
  explicit Namespace(Namespace *parent) noexcept
    : _parent{parent}
  {}

  // Named namespace constructor
  explicit Namespace(std::string name, Namespace *parent) noexcept
    : Symbol{std::move(name)}
    , _parent{parent}
  {}

  virtual ~Namespace();

  SymbolKind symbol_kind() const override;
  Namespace *parent() { return _parent; }
  const Namespace *parent() const { return _parent; }

  size_t symbols_count() const { return _symbols.size(); }
  Symbol *operator[](size_t n) { return _symbols[n].get(); }
  const Symbol *operator[](size_t n) const { return _symbols[n].get(); }
  Symbol *operator[](std::string_view name) {
    auto it = _symbol_names.find(name);
    if (it == _symbol_names.end()) {
      return nullptr;
    }
    return _symbols[it->second].get();
  }
  const Symbol *operator[](std::string_view name) const
  { return const_cast<Namespace*>(this)->operator[](name); }
  Symbol *get_symbol_recursive(std::string_view name) {
    auto it = _symbol_names.find(name);
    if (it == _symbol_names.end()) {
      if (_parent) {
        return _parent->get_symbol_recursive(name);
      }
      return nullptr;
    }
    return _symbols[it->second].get();
  }
  const Symbol *get_symbol_recursive(std::string_view name) const
  { return const_cast<Namespace*>(this)->get_symbol_recursive(name); }

  template<class TSymbol, class TStr, class... Args>
  TSymbol *add_symbol(TStr &&name, Args &&...args) {
    auto [it, inserted] = _symbol_names.emplace(name, _symbols.size());
    if (inserted) {
      auto *sym = _symbols.emplace_back(
          std::make_unique<TSymbol>(
            std::string{std::forward<TStr>(name)},
            std::forward<Args>(args)...
          )).get();
      return static_cast<TSymbol*>(sym);
    }
    return nullptr;
  }

  template<class TSymbol, class... Args>
  TSymbol *add_named_symbol(Args &&...args) {
    auto sym = std::make_unique<TSymbol>(std::forward<Args>(args)...);
    auto [it, inserted] = _symbol_names.emplace(sym->name(), _symbols.size());
    if (inserted) {
      return static_cast<TSymbol*>(
        _symbols.emplace_back(std::move(sym)).get());
    }
    return nullptr;
  }

  template<class TSymbol, class... Args>
  TSymbol *add_anon_symbol(Args &&...args) {
    return static_cast<TSymbol*>(_symbols.emplace_back(
        std::make_unique<TSymbol>(std::forward<Args>(args)...)
      ).get());
  }
};

struct Type;

struct Variable : Symbol {
private:
  Type *_type;

public:
  explicit Variable(std::string name, Type *type) noexcept
    : Symbol{std::move(name)}
    , _type{type}
  {}

  virtual ~Variable();
  Type &type() { return *_type; }
  const Type &type() const { return *_type; }
};

struct Global : Variable {
private:
  Namespace *_parent;

public:
  explicit Global(std::string name, Namespace *parent, Type *type) noexcept
    : Variable{std::move(name), type}
    , _parent{parent}
  {}

  virtual ~Global();
  SymbolKind symbol_kind() const override;
  Namespace &parent() { return *_parent; }
  const Namespace &parent() const { return *_parent; }
};

struct Scope;
struct Local : Variable {
private:
  Scope *_parent;

public:
  explicit Local(std::string name, Scope *parent, Type *type) noexcept
    : Variable{std::move(name), type}
    , _parent{parent}
  {}

  virtual ~Local();
  SymbolKind symbol_kind() const override;
  Scope &parent() { return *_parent; }
  const Scope &parent() const { return *_parent; }
};

struct StructType;
struct Field : Variable {
private:
  StructType *_parent;

public:
  explicit Field(std::string name, StructType *parent, Type *type) noexcept
    : Variable{std::move(name), type}
    , _parent{parent}
  {}

  virtual ~Field();
  SymbolKind symbol_kind() const override;
  StructType &parent() { return *_parent; }
  const StructType &parent() const { return *_parent; }
};

} // namespace lava::syn

#endif // LAVA_SYN_SYMBOL_H_
