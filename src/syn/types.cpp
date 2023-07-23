#include "lava/lava.h"
#include "lava/syn/types.h"
#include <boost/container_hash/hash.hpp>

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
  case TypeKind::Data:
    return CMP(DataType);
  case TypeKind::Int:
    return CMP(IntType);
  case TypeKind::Float:
    return CMP(FloatType);
  case TypeKind::Vector:
    return CMP(VectorType);
  case TypeKind::Pointer:
    return CMP(PointerType);
  case TypeKind::Fun:
    return CMP(FunType);
  case TypeKind::Struct:
    return CMP(StructType);
  default:
    LAVA_UNREACHABLE();
#undef CMP
  }
}

// ------------------------------------------------------------------------- //

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

TypeKind DataType::type_kind() const {
  return TypeKind::Data;
}

size_t DataType::hash() const {
  size_t seed;
  boost::hash_combine(seed, size);
  boost::hash_combine(seed, align);
  return seed;
}

bool DataType::operator==(const DataType &other) const {
  return size == other.size && align == other.align;
}

// ------------------------------------------------------------------------- //

TypeKind IntType::type_kind() const {
  return TypeKind::Int;
}

size_t IntType::hash() const {
  size_t seed{DataType::hash()};
  boost::hash_combine(seed, is_signed);
  return seed;
}

bool IntType::operator==(const IntType &other) const {
  return DataType::operator==(other) && is_signed == other.is_signed;
}

// ------------------------------------------------------------------------- //

TypeKind FloatType::type_kind() const {
  return TypeKind::Float;
}

bool FloatType::operator==(const FloatType &other) const {
  return DataType::operator==(other);
}

// ------------------------------------------------------------------------- //

TypeKind VectorType::type_kind() const {
  return TypeKind::Vector;
}

bool VectorType::operator==(const VectorType &other) const {
  return DataType::operator==(other);
}

// ------------------------------------------------------------------------- //

TypeKind PointerType::type_kind() const {
  return TypeKind::Pointer;
}

size_t PointerType::hash() const {
  return pointee->hash();
}

bool PointerType::operator==(const PointerType &other) const {
  return pointee->operator==(*other.pointee);
}

// ------------------------------------------------------------------------- //

TypeKind FunType::type_kind() const {
  return TypeKind::Fun;
}

size_t FunType::hash() const {
  size_t seed = 0;
  boost::hash_combine(seed, ret->hash());
  for (auto const *arg : args) {
    boost::hash_combine(seed, arg->hash());
  }
  return seed;
}

bool FunType::operator==(const FunType &other) const {
  if (*ret != *other.ret) {
    return false;
  }
  if (args.size() != other.args.size()) {
    return false;
  }
  for (unsigned i = 0; i < args.size(); ++i) {
    if (*args[i] != *other.args[i]) {
      return false;
    }
  }

  return true;
}

// ------------------------------------------------------------------------- //

TypeKind StructType::type_kind() const {
  return TypeKind::Struct;
}

size_t StructType::hash() const {
  return std::hash<size_t>{}(reinterpret_cast<size_t>(this));
}

bool StructType::operator==(const StructType &other) const {
  return this == &other;
}
