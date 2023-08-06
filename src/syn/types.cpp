#include "lava/lava.h"
#include "lava/syn/types.h"
#include <boost/container_hash/hash.hpp>
#include <cassert>

using namespace lava::syn;

Type::~Type() {}

SymbolKind Type::symbol_kind() const {
  return SymbolKind::Type;
}

bool Type::operator==(const Type &other) const {
  if (this == &other) {
    return true;
  }

  auto kind = type_kind();
  if (kind != other.type_kind()) {
    return false;
  }

  switch (kind) {
#define CMP(T) static_cast<const T&>(*this) == static_cast<const T&>(other)
  case TypeKind::Never:
    return CMP(NeverType);
  case TypeKind::Void:
    return CMP(VoidType);
  case TypeKind::Int:
    return CMP(IntType);
  case TypeKind::Float:
    return CMP(FloatType);
  case TypeKind::Vector:
    return CMP(VectorType);
  case TypeKind::Pointer:
    return CMP(PointerType);
  case TypeKind::Function:
    return CMP(FunctionType);
  case TypeKind::Struct:
    return CMP(StructType);
  default:
    LAVA_UNREACHABLE();
#undef CMP
  }
}

// ------------------------------------------------------------------------- //

size_t NeverType::size() const {
  return 0;
}

size_t NeverType::align() const {
  return 0;
}

TypeKind NeverType::type_kind() const {
  return TypeKind::Never;
}

size_t NeverType::hash() const {
  return 0;
}

bool NeverType::operator==(const NeverType &other) const {
  return true;
}

// ------------------------------------------------------------------------- //

size_t VoidType::size() const {
  return 0;
}

size_t VoidType::align() const {
  return 0;
}

TypeKind VoidType::type_kind() const {
  return TypeKind::Void;
}

size_t VoidType::hash() const {
  return 1;
}

bool VoidType::operator==(const VoidType &other) const {
  return true;
}

// ------------------------------------------------------------------------- //

size_t IntType::size() const {
  return _size;
}

size_t IntType::align() const {
  return _size;
}

TypeKind IntType::type_kind() const {
  return TypeKind::Int;
}

size_t IntType::hash() const {
  size_t seed = 0;
  boost::hash_combine(seed, _size);
  boost::hash_combine(seed, _is_signed);
  return seed;
}

bool IntType::operator==(const IntType &other) const {
  return _size == other._size && _is_signed == other._is_signed;
}

// ------------------------------------------------------------------------- //

size_t FloatType::size() const {
  return _size;
}

size_t FloatType::align() const {
  return _size;
}

TypeKind FloatType::type_kind() const {
  return TypeKind::Float;
}

size_t FloatType::hash() const {
  return std::hash<size_t>{}(_size);
}

bool FloatType::operator==(const FloatType &other) const {
  return _size == other._size;
}

// ------------------------------------------------------------------------- //

size_t VectorType::size() const {
  return _size;
}

size_t VectorType::align() const {
  return _size;
}

TypeKind VectorType::type_kind() const {
  return TypeKind::Vector;
}

size_t VectorType::hash() const {
  return std::hash<size_t>{}(_size);
}

bool VectorType::operator==(const VectorType &other) const {
  return _size == other._size;
}

// ------------------------------------------------------------------------- //

size_t PointerType::size() const {
  return sizeof(void*);
}

size_t PointerType::align() const {
  return alignof(void*);
}

TypeKind PointerType::type_kind() const {
  return TypeKind::Pointer;
}

size_t PointerType::hash() const {
  size_t seed = 0x1f2e3d4c;
  boost::hash_combine(seed, _pointee->hash());
  return seed;
}

bool PointerType::operator==(const PointerType &other) const {
  return _pointee->operator==(*other._pointee);
}

// ------------------------------------------------------------------------- //

size_t FunctionType::size() const {
  return 0;
}

size_t FunctionType::align() const {
  return 0;
}

TypeKind FunctionType::type_kind() const {
  return TypeKind::Function;
}

size_t FunctionType::hash() const {
  size_t seed = 0;
  boost::hash_combine(seed, _return_type->hash());
  for (auto const *arg : _arg_types) {
    boost::hash_combine(seed, arg->hash());
  }
  return seed;
}

bool FunctionType::operator==(const FunctionType &other) const {
  if (*_return_type != *other._return_type) {
    return false;
  }
  if (_arg_types.size() != other._arg_types.size()) {
    return false;
  }
  for (unsigned i = 0; i < _arg_types.size(); ++i) {
    if (*_arg_types[i] != *other._arg_types[i]) {
      return false;
    }
  }

  return true;
}

// ------------------------------------------------------------------------- //

StructType::StructType(std::string name, std::vector<StructField> fields)
  : Type{std::move(name)}
  , _fields{std::move(fields)}
  , _size{0}
  , _align{1}
{
  for (size_t i = 0; i < _fields.size(); ++i) {
    auto const &field = _fields[i];
    [[maybe_unused]] auto inserted =
      _field_names.emplace(field.name, i).second;
    assert(inserted && "Duplicate field name");
    auto align = field.type->align();
    if (align > _align) {
      _align = align;
    }
    _size = (_size + align - 1) & -align;
    _size += field.type->size();
  }
}

size_t StructType::size() const {
  return _size;
}

size_t StructType::align() const {
  return _align;
}

TypeKind StructType::type_kind() const {
  return TypeKind::Struct;
}

size_t StructType::hash() const {
  return std::hash<size_t>{}(reinterpret_cast<size_t>(this));
}

bool StructType::operator==(const StructType &other) const {
  return this == &other;
}
