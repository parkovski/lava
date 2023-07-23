#include "lava/syn/symtab.h"
#include <cassert>

using namespace lava::syn;

Variable::~Variable() {}

// ------------------------------------------------------------------------- //

std::pair<Variable*, bool> Scope::add_variable(std::unique_ptr<Variable> var) {
  auto [it, inserted] = _var_names.emplace(var->name, _vars.size());
  if (inserted) {
    _vars.emplace_back(std::move(var));
    return std::make_pair(_vars.back().get(), true);
  }
  return std::make_pair(_vars[it->second].get(), false);
}

Variable *Scope::get_variable(std::string_view name) {
  auto it = _var_names.find(name);
  if (it == _var_names.end()) {
    if (_parent) {
      return _parent->get_variable(name);
    }
    return nullptr;
  }
  return _vars[it->second].get();
}

std::pair<Type *, bool> Scope::add_named_type(Type *type) {
  assert(!type->get_name().empty());
  [[maybe_unused]] auto [it, inserted] =
    _type_names.emplace(type->get_name(), type);
  if (inserted) {
    return std::make_pair(type, true);
  } else {
    return std::make_pair(it->second, false);
  }
}

Type *Scope::get_type(std::string_view name) {
  auto it = _type_names.find(name);
  if (it == _type_names.end()) {
    if (_parent) {
      return _parent->get_type(name);
    }
    return nullptr;
  }
  return it->second;
}

Scope *Scope::add_scope(std::string name) {
  if (!name.empty()) {
    if (get_scope(name) != nullptr) {
      return nullptr;
    }
  }
  auto *scope = _scopes.emplace_back(
    std::unique_ptr<Scope>{new Scope{this, std::move(name)}}).get();
  if (!scope->name().empty()) {
    _scope_names.emplace(scope->name(), _scopes.size() - 1);
  }
  return scope;
}

Scope *Scope::get_scope(std::string_view name) {
  auto it = _scope_names.find(name);
  if (it == _scope_names.end()) {
    return nullptr;
  }
  return _scopes[it->second].get();
}

// ------------------------------------------------------------------------- //

void SymbolTable::add_named_type(Type *type) {
  [[maybe_unused]] auto inserted = _global.add_named_type(type).second;
  assert(inserted);
  inserted = _types.emplace(type).second;
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
  for (auto *type : _types) {
    delete type;
  }
}

void SymbolTable::add_type(Type *type) {
  [[maybe_unused]] bool inserted = _types.insert(type).second;
  assert(inserted);
}

Type *SymbolTable::find_canonical_type(Type *type) {
  auto it = _types.find(type);
  if (it == _types.end()) {
    add_type(type);
    return type;
  } else {
    delete type;
    return *it;
  }
}
