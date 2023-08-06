#include "lava/syn/symtab.h"
#include <cassert>

using namespace lava::syn;

Symbol::~Symbol() {}

// ------------------------------------------------------------------------- //

Namespace::~Namespace() {}

SymbolKind Namespace::symbol_kind() const {
  return SymbolKind::Namespace;
}

// ------------------------------------------------------------------------- //

std::string FunctionType::make_name(Type *return_type,
                                    const std::vector<Type*> &arg_types) {
  std::string name{"fun("};
  if (!arg_types.empty()) {
    name.append(arg_types[0]->name());
  }
  for (size_t i = 1; i < arg_types.size(); ++i) {
    name.append(", ").append(arg_types[i]->name());
  }
  name.append(") -> ");
  name.append(return_type->name());
  return name;
}

// ------------------------------------------------------------------------- //

void SymbolTable::enter_initial_types() {
  _global.add_named_symbol<NeverType>();

  _global.add_named_symbol<VoidType>();

  _global.add_symbol<IntType>("uint8", 8, false);
  _global.add_symbol<IntType>("int8", 8, true);
  _global.add_symbol<IntType>("uint16", 16, false);
  _global.add_symbol<IntType>("int16", 16, true);
  _global.add_symbol<IntType>("uint32", 32, false);
  _global.add_symbol<IntType>("int32", 32, true);
  _global.add_symbol<IntType>("uint64", 64, false);
  _global.add_symbol<IntType>("int64", 64, true);

  _global.add_symbol<FloatType>("float32", 32);
  _global.add_symbol<FloatType>("float64", 64);
}

SymbolTable::SymbolTable()
  : _global{}
{
  enter_initial_types();
}

SymbolTable::~SymbolTable() {}

Type *SymbolTable::find_unnamed_type(std::unique_ptr<Type> type) {
  auto it = _unnamed_types.find(type);
  if (it == _unnamed_types.end()) {
    return _unnamed_types.emplace(std::move(type)).first->get();
  } else {
    return it->get();
  }
}
