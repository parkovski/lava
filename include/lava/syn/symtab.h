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

  std::unordered_map<std::string_view, Symbol*> _symbols;

  std::vector<std::unique_ptr<Variable>> _vars;
  std::unordered_map<std::string_view, size_t> _var_names;

  std::unordered_map<std::string_view, Type*> _type_names;

  std::vector<std::unique_ptr<Scope>> _scopes;
  std::unordered_map<std::string_view, size_t> _scope_names;

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

  Symbol *get_symbol(std::string_view name);

  std::pair<Variable*, bool> add_variable(std::unique_ptr<Variable> var);
  size_t variable_count() const { return _vars.size(); }
  Variable *get_variable(size_t n) { return _vars[n].get(); }
  const Variable *get_variable(size_t n) const { return _vars[n].get(); }
  // Searches parent scopes recursively.
  Variable *get_variable(std::string_view name);
  // Searches parent scopes recursively.
  const Variable *get_variable(std::string_view name) const
  { return const_cast<Scope*>(this)->get_variable(name); }

  std::pair<Type *, bool> add_named_type(Type *type);
  // Searches parent scopes recursively.
  Type *get_type(std::string_view name);
  // Searches parent scopes recursively.
  const Type *get_type(std::string_view name) const
  { return const_cast<Scope*>(this)->get_type(name); }

  Scope *add_scope(std::string name = "");
  size_t scope_count() const { return _scopes.size(); }
  Scope *get_scope(size_t n) { return _scopes[n].get(); }
  const Scope *get_scope(size_t n) const { return _scopes[n].get(); }
  Scope *get_scope(std::string_view name);
  const Scope *get_scope(std::string_view name) const
  { return const_cast<Scope*>(this)->get_scope(name); }
};

struct Function : Variable {
  Scope arg_scope;

  virtual ~Function();
  SymbolKind symbol_kind() const override;
};

struct SymbolTable {
private:
  std::unordered_set<Type*, detail::ptrhash_type, detail::ptreq_type> _types;
  Scope _global;

  void add_named_type(Type *type);
  void enter_initial_types();

public:
  explicit SymbolTable();
  ~SymbolTable();

  // `type` must be a globally unique type, otherwise use
  // `find_canonical_type`.
  void add_type(Type *type);

  // Takes ownership of `type`, either deleting it or inserting it into the
  // global type table.
  Type *find_canonical_type(Type *type);

  Scope &global_scope() { return _global; }
  const Scope &global_scope() const { return _global; }
};

} // namespace lava::syn

#endif // LAVA_SYN_SYMTAB_H_
