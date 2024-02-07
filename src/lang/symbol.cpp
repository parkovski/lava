#include "lava/lava.h"
#include "lava/lang/symbol.h"
#include <stdexcept>

using namespace lava::lang;

Symbol::~Symbol() {}

Namespace::~Namespace() {}

Symbol *Namespace::get(const SymbolPath &path) {
  if (path.empty()) {
    return nullptr;
  }
  Symbol *symbol = get(path[0]);
  for (size_t i = 1; i < path.size(); ++i) {
    auto *ns = dynamic_cast<Namespace*>(symbol);
    if (ns) {
      symbol = ns->get(path[i]);
      if (!symbol) {
        return nullptr;
      }
    } else {
      return nullptr;
    }
  }
  return symbol;
}

Symbol *Namespace::getrec(InternString name) {
  auto *symbol = get(name);
  if (!symbol) {
    if (!_using.empty()) {
      for (auto *using_ns : _using) {
        auto *symbol = using_ns->get(name);
        if (symbol) {
          return symbol;
        }
      }
    }
    if (_parent) {
      return _parent->getrec(name);
    }
  }
  return symbol;
}

Symbol *Namespace::getrec(const SymbolPath &path) {
  auto *symbol = get(path);
  if (!symbol) {
    if (!_using.empty()) {
      for (auto *using_ns : _using) {
        auto *symbol = using_ns->get(path);
        if (symbol) {
          return symbol;
        }
      }
    }
    if (_parent) {
      return _parent->getrec(path);
    }
  }
  return symbol;
}

Type::~Type() {}

bool Type::operator==(const Type &other) const {
  auto kind = this->kind();
  if (kind != other.kind()) {
    return false;
  }

  switch (kind) {
#define CASE(Kind) \
  case TypeKind::Kind: \
    return *static_cast<const Kind##Type*>(this) == \
      static_cast<const Kind##Type&>(other)

  CASE(Never);
  CASE(Any);
  CASE(Void);
  CASE(Bool);
  CASE(Int);
  CASE(Float);
  CASE(Pointer);
  CASE(NullPointer);
  CASE(Array);
  CASE(Struct);
  CASE(Function);

#undef CASE
  }
}

NeverType::~NeverType() {}

TypeKind NeverType::kind() const {
  return TypeKind::Never;
}

size_t NeverType::hash() const {
  return 0x80000001;
}

AnyType::~AnyType() {}

TypeKind AnyType::kind() const {
  return TypeKind::Any;
}

size_t AnyType::hash() const {
  return 0x80000002;
}

VoidType::~VoidType() {}

TypeKind VoidType::kind() const {
  return TypeKind::Void;
}

size_t VoidType::hash() const {
  return 0x80000003;
}

BoolType::~BoolType() {}
TypeKind BoolType::kind() const {
  return TypeKind::Bool;
}

size_t BoolType::hash() const {
  return 0x8000000B;
}

IntType::~IntType() {}

TypeKind IntType::kind() const {
  return TypeKind::Int;
}

size_t IntType::hash() const {
  size_t hash = 0x80000004;
  boost::hash_combine(hash, size);
  boost::hash_combine(hash, is_signed);
  return hash;
}

bool IntType::operator==(const IntType &other) const {
  return size == other.size && is_signed == other.is_signed;
}

FloatType::~FloatType() {}

TypeKind FloatType::kind() const {
  return TypeKind::Float;
}

size_t FloatType::hash() const {
  size_t hash = 0x80000005;
  boost::hash_combine(hash, size);
  return hash;
}

bool FloatType::operator==(const FloatType &other) const {
  return size == other.size;
}

unsigned PointerType::TargetPointerSize = 0;

PointerType::~PointerType() {}

TypeKind PointerType::kind() const {
  return TypeKind::Pointer;
}

size_t PointerType::hash() const {
  size_t hash = 0x80000006;
  if (pointed_at) {
    boost::hash_combine(hash, pointed_at->hash());
  }
  return hash;
}

bool PointerType::operator==(const PointerType &other) const {
  return *pointed_at == *other.pointed_at;
}

NullPointerType::~NullPointerType() {}

TypeKind NullPointerType::kind() const {
  return TypeKind::NullPointer;
}

unsigned ArrayType::aligned_size(unsigned eltsize, unsigned eltalign,
                                 unsigned size) {
  return size * LAVA_ALIGN_CEIL(eltsize, eltalign);
}

ArrayType::ArrayType(const DataType *element_type, unsigned array_length)
  : DataType{
    aligned_size(element_type->size, element_type->align, array_length),
    element_type->align
  }
  , element_type{element_type}
  , array_length{array_length}
{}

ArrayType::~ArrayType() {}

TypeKind ArrayType::kind() const {
  return TypeKind::Array;
}

size_t ArrayType::hash() const {
  size_t hash = 0x80000007;
  boost::hash_combine(hash, element_type->hash());
  boost::hash_combine(hash, array_length);
  return hash;
}

bool ArrayType::operator==(const ArrayType &other) const {
  return array_length == other.array_length &&
    *element_type == *other.element_type;
}

unsigned StructType::get_align(const FieldVector &fields) {
  unsigned maxalign = 1;
  for (size_t i = 0; i < fields.size(); ++i) {
    if (fields[i].type->align > maxalign) {
      maxalign = fields[i].type->align;
    }
  }
  return maxalign;
}

unsigned StructType::align_fields(FieldVector &fields) {
  if (fields.empty()) {
    return 0;
  }

  unsigned offset = 0;
  for (size_t i = 0; i < fields.size(); ++i) {
    offset = LAVA_ALIGN_CEIL(offset, fields[i].type->align);
    fields[i].offset = offset;
    offset += fields[i].type->size;
  }
  return offset;
}

StructType::~StructType() {}

TypeKind StructType::kind() const {
  return TypeKind::Struct;
}

size_t StructType::hash() const {
  size_t hash = 0x80000009;
  for (auto const &field : fields) {
    boost::hash_combine(hash, field.name);
    boost::hash_combine(hash, field.type->hash());
  }
  return hash;
}

bool StructType::operator==(const StructType &other) const {
  if (fields.size() != other.fields.size()) {
    return false;
  }
  for (size_t i = 0; i < fields.size(); ++i) {
    if (fields[i].name != other.fields[i].name) {
      return false;
    }
    if (*fields[i].type != *other.fields[i].type) {
      return false;
    }
  }
  return true;
}

FunctionType::~FunctionType() {}

TypeKind FunctionType::kind() const {
  return TypeKind::Function;
}

size_t FunctionType::hash() const {
  size_t hash = 0x8000000A;
  boost::hash_combine(hash, return_type->hash());
  for (auto const &arg : arg_types) {
    boost::hash_combine(hash, arg.type->hash());
    boost::hash_combine(hash, arg.name);
  }
  return hash;
}

bool FunctionType::operator==(const FunctionType &other) const {
  if (arg_types.size() != other.arg_types.size()) {
    return false;
  }
  if (*return_type != *other.return_type) {
    return false;
  }
  for (size_t i = 0; i < arg_types.size(); ++i) {
    if (arg_types[i].type != other.arg_types[i].type ||
        arg_types[i].name != other.arg_types[i].name) {
      return false;
    }
  }
  return true;
}

bool FunctionType::are_types_same(const FunctionType &other) const {
  if (arg_types.size() != other.arg_types.size()) {
    return false;
  }
  if (*return_type != *other.return_type) {
    return false;
  }
  for (size_t i = 0; i < arg_types.size(); ++i) {
    if (arg_types[i].type != other.arg_types[i].type) {
      return false;
    }
  }
  return true;
}

void Function::add_args() {
  for (auto const &arg : _type->arg_types) {
    _args_ns.add(std::make_unique<Variable>(arg.name, arg.type));
  }
}

Function::Function(InternString name, const FunctionType *type,
                   Namespace &current_ns)
  : Symbol{name}
  , _type{type}
  , _args_ns{current_ns}
  , _locals_ns{_args_ns}
{
  add_args();
}

void Function::set_type(const FunctionType *type) {
  _type = type;
  _args_ns.clear();
  add_args();
}

void SymbolTable::add_base_types() {
  _global_ns.add(std::make_unique<TypeAlias>(intern("never"), &never_type()));
  _global_ns.add(std::make_unique<TypeAlias>(intern("any"), &any_type()));
  _global_ns.add(std::make_unique<TypeAlias>(intern("void"), &void_type()));
  _global_ns.add(std::make_unique<TypeAlias>(intern("bool"), &bool_type()));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("null"),
    &null_pointer_type()
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("int8"),
    &int_type(1, true)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("uint8"),
    &int_type(1, false)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("int16"),
    &int_type(2, true)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("uint16"),
    &int_type(2, false)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("int32"),
    &int_type(4, true)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("uint32"),
    &int_type(4, false)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("int64"),
    &int_type(8, true)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("uint64"),
    &int_type(8, false)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("int128"),
    &int_type(16, true)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("uint128"),
    &int_type(16, false)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("int256"),
    &int_type(32, true)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("uint256"),
    &int_type(32, false)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("float"),
    &float_type(4)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("double"),
    &float_type(8)
  ));

  assert(PointerType::TargetPointerSize != 0);
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("int"),
    &int_type(PointerType::TargetPointerSize, true)
  ));
  _global_ns.add(std::make_unique<TypeAlias>(
    intern("uint"),
    &int_type(PointerType::TargetPointerSize, false)
  ));
}

SymbolTable::SymbolTable()
  : _the_string{}
  , _string_map{}
  , _anon_index{0}
  , _global_ns{}
  , _never_type{}
  , _any_type{}
  , _void_type{}
  , _bool_type{}
  , _null_pointer_type{}
  , _int8_type{1, true}
  , _uint8_type{1, false}
  , _int16_type{2, true}
  , _uint16_type{2, false}
  , _int32_type{4, true}
  , _uint32_type{4, false}
  , _int64_type{8, true}
  , _uint64_type{8, false}
  , _int128_type{16, true}
  , _uint128_type{16, false}
  , _int256_type{32, true}
  , _uint256_type{32, false}
  , _float_type{FloatType::float_type()}
  , _double_type{FloatType::double_type()}
{
  add_base_types();
}

InternString SymbolTable::intern(std::string_view str) {
  auto it = _string_map.find(str);
  if (it == _string_map.end()) {
    InternString istr{ _the_string.size(), str.size() };
    _the_string.append(str);
    _string_map.emplace(str, istr);
    return istr;
  } else {
    return it->second;
  }
}

std::string_view SymbolTable::get_string(InternString str) {
  return std::string_view(_the_string).substr(str.offset, str.size);
}

InternString SymbolTable::get_anon_name() {
  std::string name{"_$_"};
  size_t index = _anon_index;
  while (index) {
    name.append(1, '0' + (index % 10));
    index /= 10;
  }
  return intern(name);
}

const IntType &
SymbolTable::int_type(unsigned size, bool is_signed) const {
  switch (size) {
  case 1: return is_signed ? _int8_type : _uint8_type;
  case 2: return is_signed ? _int16_type : _uint16_type;
  case 4: return is_signed ? _int32_type : _uint32_type;
  case 8: return is_signed ? _int64_type : _uint64_type;
  case 16: return is_signed ? _int128_type : _uint128_type;
  case 32: return is_signed ? _int256_type : _uint256_type;
  default: throw std::domain_error("expected size power of 2, <= 32");
  }
}

const IntType &SymbolTable::int_type(bool is_signed) const {
  return int_type(PointerType::TargetPointerSize, is_signed);
}

const FloatType &
SymbolTable::float_type(unsigned size) const {
  if (size == 4) return _float_type;
  if (size == 8) return _double_type;
  throw std::domain_error("expected size 4 or 8");
}

const PointerType &
SymbolTable::pointer_type(PointerType &&pointer_type) const {
  return *_pointer_types.emplace(std::move(pointer_type)).first;
}

const ArrayType &SymbolTable::array_type(ArrayType &&array_type) const {
  return *_array_types.emplace(std::move(array_type)).first;
}

const StructType &SymbolTable::struct_type(StructType &&struct_type) const {
  return *_struct_types.emplace(std::move(struct_type)).first;
}

const FunctionType &
SymbolTable::function_type(FunctionType &&function_type) const {
  return *_function_types.emplace(std::move(function_type)).first;
}
