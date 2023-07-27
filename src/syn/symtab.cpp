#include "lava/syn/symtab.h"
#include <cassert>

using namespace lava::syn;

Symbol::~Symbol() {}

// ------------------------------------------------------------------------- //

Variable::~Variable() {}

SymbolKind Variable::symbol_kind() const {
  return SymbolKind::Variable;
}

// ------------------------------------------------------------------------- //

Scope::~Scope() {}

SymbolKind Scope::symbol_kind() const {
  return SymbolKind::Scope;
}

std::pair<Symbol*, bool> Scope::add_symbol(std::unique_ptr<Symbol> symbol) {
  auto [it, inserted] = _symbols.emplace(symbol->name(), std::move(symbol));
  if (inserted) {
    if (it->second->symbol_kind() == SymbolKind::Variable) {
      _vars.emplace_back(static_cast<Variable*>(it->second.get()));
    }
    return std::make_pair(it->second.get(), true);
  }
  return std::make_pair(it->second.get(), false);
}

Scope *Scope::add_scope(std::string name) {
  if (name.empty()) {
    return _unnamed_scopes
      .emplace_back(std::unique_ptr<Scope>{new Scope{this}}).get();
  }
  auto it = _symbols.find(name);
  if (it == _symbols.end()) {
    auto scope = std::unique_ptr<Scope>{new Scope(this, std::move(name))};
    return static_cast<Scope*>(
      _symbols.emplace(scope->name(), std::move(scope)).first->second.get());
  }
  return nullptr;
}

Symbol *Scope::get_symbol(std::string_view name) {
  auto it = _symbols.find(name);
  if (it == _symbols.end()) {
    if (_parent) {
      return _parent->get_symbol(name);
    }
    return nullptr;
  }
  return it->second.get();
}

Symbol *Scope::get_symbol_nonrec(std::string_view name) {
  auto it = _symbols.find(name);
  if (it == _symbols.end()) {
    return nullptr;
  }
  return it->second.get();
}

// ------------------------------------------------------------------------- //

Function::~Function() {}

SymbolKind Function::symbol_kind() const {
  return SymbolKind::Function;
}

// ------------------------------------------------------------------------- //

void SymbolTable::add_named_type(Type *type) {
  [[maybe_unused]] auto inserted =
    _global.add_symbol(std::unique_ptr<Symbol>{type}).second;
  assert(inserted);
}

void SymbolTable::enter_initial_types() {
  add_named_type(new NeverType);

  add_named_type(new VoidType);

  add_named_type(new IntType{"uint8", 8, false});
  add_named_type(new IntType{"int8", 8, true});
  add_named_type(new IntType{"uint16", 16, false});
  add_named_type(new IntType{"int16", 16, true});
  add_named_type(new IntType{"uint32", 32, false});
  add_named_type(new IntType{"int32", 32, true});
  add_named_type(new IntType{"uint64", 64, false});
  add_named_type(new IntType{"int64", 64, true});

  add_named_type(new FloatType{"float32", 32});
  add_named_type(new FloatType{"float64", 64});
}

SymbolTable::SymbolTable()
  : _global{this}
{
  enter_initial_types();
}

SymbolTable::~SymbolTable() {
  for (auto *type : _unnamed_types) {
    delete type;
  }
}

Type *SymbolTable::find_unnamed_type(std::unique_ptr<Type> type) {
  auto it = _unnamed_types.find(type.get());
  if (it == _unnamed_types.end()) {
    return *_unnamed_types.emplace(type.release()).first;
  } else {
    return *it;
  }
}
