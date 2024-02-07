#include "lava/lang/firstpass.h"
#include <stdexcept>

using namespace lava::lang;

FirstPass::FirstPass(SymbolTable &symtab)
  : _symtab{&symtab}
  , _current_ns{&symtab.global_namespace()}
{}

const FunctionType &FirstPass::get_function_type(const FunItemBase &item) {
  const DataType *return_type;
  if (item.return_type()) {
    TypeVisitor return_type_visitor{*_symtab, *_current_ns};
    return_type_visitor.NodeVisitor::visit(*item.return_type());
    return_type = dynamic_cast<const DataType*>(return_type_visitor.type);
    if (!return_type) {
      throw std::runtime_error{"Return type is not a DataType"};
    }
  } else {
    return_type = &_symtab->void_type();
  }

  FunctionType::ArgVector args;
  for (auto const &arg : item.args()) {
    TypeVisitor arg_type_visitor{*_symtab, *_current_ns};
    arg_type_visitor.NodeVisitor::visit(*arg.value.type());
    auto type = dynamic_cast<const DataType*>(arg_type_visitor.type);
    if (!type) {
      throw std::runtime_error{"Arg is not a DataType"};
    }
    args.emplace_back(_symtab->intern(arg.value.name()), type);
  }

  auto const &type = _symtab->function_type(
    FunctionType{return_type, std::move(args)}
  );
  return type;
}

void FirstPass::visit(const FunDeclItem &item) {
  auto name = _symtab->intern(item.name());
  auto const &type = get_function_type(item);
  if (!_current_ns->add(
      std::make_unique<Function>(name, &type, *_current_ns)
     )) {
    throw std::runtime_error{"Duplicate function definition"};
  }
}

void FirstPass::visit(const FunDefItem &item) {
  auto name = _symtab->intern(item.name());
  auto const &type = get_function_type(item);
  if (!_current_ns->add(
      std::make_unique<Function>(name, &type, *_current_ns)
     )) {
    auto fn = dynamic_cast<Function*>(_current_ns->get(name));
    if (!fn || !type.are_types_same(*fn->type())) {
      throw std::runtime_error{"Function declaration/definition mismatch"};
    } else {
      // Set the type to have correct arg names.
      fn->set_type(&type);
    }
  }
}

void TypeVisitor::visit(const IdentExpr &ident) {
  auto name = _symtab->intern(ident.value());
  auto *sym = _current_ns->get(name);
  if (auto *ns = dynamic_cast<Namespace*>(sym)) {
    _current_ns = ns;
  } else if (auto *typealias = dynamic_cast<TypeAlias*>(sym)) {
    type = typealias->type;
  } else {
    throw std::runtime_error{"Unknown ident kind"};
  }
}

void TypeVisitor::visit(const BinaryExpr &binary) {
  if (binary.op() == TkDot) {
    NodeVisitor::visit(*binary.left());
    assert(type == nullptr && _current_ns);
    NodeVisitor::visit(*binary.right());
    assert(type);
  } else {
    throw std::runtime_error{"Expression not supported for type"};
  }
}
