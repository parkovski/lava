#ifndef LAVA_SYN_SYMTAB_H_
#define LAVA_SYN_SYMTAB_H_

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <string_view>
#include <memory>
#include <boost/container/small_vector.hpp>
#include "symbol.h"
#include "types.h"

namespace lava::syn {

namespace detail {
  struct ptrhash_type {
    const size_t operator()(const Type *value) const {
      return std::hash<Type>{}(*value);
    }
  };

  struct ptreq_type {
    const bool operator()(const Type *a, const Type *b) const {
      return *a == *b;
    }
  };
}

enum class Op {
  Assign,
  Call,
  Scope,
};

struct Instruction {
  Op op;
  boost::container::small_vector<size_t, 3> args;
};

struct Variable : Symbol {
private:
  Type *_type;
  // std::vector<Instruction> _init;

public:
  explicit Variable(Type *type, std::string name) noexcept
    : Symbol{std::move(name)}
    , _type{type}
  {}

  virtual ~Variable();

  Type *type() { return _type; }
  const Type *type() const { return _type; }
  SymbolKind symbol_kind() const override;
};

struct SymbolTable;
struct Scope : Symbol {
private:
  SymbolTable *_symtab;
  Scope *_parent;

  std::unordered_map<std::string_view, std::unique_ptr<Symbol>> _symbols;
  std::vector<Variable*> _vars;

  explicit Scope(Scope *parent, std::string name = {}) noexcept
    : Symbol{std::move(name)}
    , _symtab{parent->_symtab}
    , _parent{parent}
  {}

public:
  // Constructor for global scope only.
  explicit Scope(SymbolTable *symtab) noexcept
    : Symbol{{}}
    , _symtab{symtab}
    , _parent{nullptr}
  {}

  virtual ~Scope();

  SymbolKind symbol_kind() const override;

  SymbolTable &symbol_table() { return *_symtab; }
  const SymbolTable &symbol_table() const { return *_symtab; }

  Scope *parent() { return _parent; }
  const Scope *parent() const { return _parent; }

  std::pair<Symbol*, bool> add_symbol(std::unique_ptr<Symbol> symbol);
  template<class T, class... Args>
  T* add_symbol(std::string name, Args &&...args) {
    auto it = _symbols.find(name);
    if (it != _symbols.end()) {
      return nullptr;
    }
    auto symbol = std::make_unique<T>(
      std::move(name), std::forward<Args>(args)...);
    it = _symbols.emplace(symbol->name(), std::move(symbol)).first;
    return static_cast<T*>(it->second.get());
  }
  Scope *add_scope(std::string name);
  // Searches parent scopes recursively.
  Symbol *get_symbol(std::string_view name);
  // Searches parent scopes recursively.
  const Symbol *get_symbol(std::string_view name) const
  { return const_cast<Scope*>(this)->get_symbol(name); }

  size_t variable_count() const { return _vars.size(); }
  Variable *get_variable(size_t n) { return _vars[n]; }
  const Variable *get_variable(size_t n) const { return _vars[n]; }
};

struct Function : Variable {
  Scope arg_scope;

  virtual ~Function();
  SymbolKind symbol_kind() const override;
};

struct SymbolTable {
private:
  std::unordered_set<Type*, detail::ptrhash_type, detail::ptreq_type>
    _unnamed_types;
  Scope _global;

  void add_named_type(Type *type);
  void enter_initial_types();

public:
  explicit SymbolTable();
  ~SymbolTable();

  // Takes ownership of `type`, either deleting it or inserting it into the
  // global type table.
  Type *find_unnamed_type(std::unique_ptr<Type> type);

  Scope &global_scope() { return _global; }
  const Scope &global_scope() const { return _global; }
};

} // namespace lava::syn

#endif // LAVA_SYN_SYMTAB_H_
