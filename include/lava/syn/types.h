#ifndef LAVA_SYN_TYPES_H_
#define LAVA_SYN_TYPES_H_

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <string_view>
#include "symbol.h"

namespace lava::syn {

enum class TypeKind {
  Never,
  Void,
  Int,
  Float,
  Vector,
  Pointer,
  Function,
  Struct,
};

struct Type : Symbol {
public:
  explicit Type() noexcept
  {}

  explicit Type(std::string name) noexcept
    : Symbol{std::move(name)}
  {}

  virtual ~Type() = 0;
  SymbolKind symbol_kind() const override;

  virtual size_t size() const = 0;
  virtual size_t align() const = 0;
  virtual TypeKind type_kind() const = 0;
  virtual size_t hash() const = 0;
  bool operator==(const Type &other) const;
};

struct NeverType final : Type {
  explicit NeverType()
    : Type{"never"}
  {}

  size_t size() const override;
  size_t align() const override;
  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const NeverType &other) const;
};

struct VoidType final : Type {
  explicit VoidType()
    : Type{"void"}
  {}

  size_t size() const override;
  size_t align() const override;
  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const VoidType &other) const;
};

struct IntType final : Type {
private:
  size_t _size;
  bool _is_signed;

public:
  explicit IntType(std::string name, size_t size, bool is_signed) noexcept
    : Type{std::move(name)}
    , _size{size}
    , _is_signed{is_signed}
  {}

  size_t size() const override;
  size_t align() const override;
  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const IntType &other) const;
  bool is_signed() const { return _is_signed; }
};

struct FloatType final : Type {
private:
  size_t _size;

public:
  explicit FloatType(std::string name, size_t size) noexcept
    : Type{std::move(name)}
    , _size{size}
  {}

  size_t size() const override;
  size_t align() const override;
  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const FloatType &other) const;
};

struct VectorType final : Type {
private:
  size_t _size;

public:
  explicit VectorType(std::string name, size_t size)
    noexcept
    : Type{std::move(name)}
    , _size{size}
  {}

  size_t size() const override;
  size_t align() const override;
  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const VectorType &other) const;
};

struct PointerType final : Type {
private:
  Type *_pointee;

public:
  explicit PointerType(Type *pointee)
    : Type{"*" + pointee->name()}
    , _pointee{pointee}
  {}

  size_t size() const override;
  size_t align() const override;
  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const PointerType &other) const;
  Type *pointee() { return _pointee; }
  const Type *pointee() const { return _pointee; }
};

struct FunctionType final : Type {
private:
  Type *_return_type;
  std::vector<Type*> _arg_types;

  static std::string make_name(Type *return_type,
                               const std::vector<Type*> &arg_types);

public:
  FunctionType(Type *return_type, std::vector<Type*> arg_types)
    : Type{make_name(return_type, arg_types)}
    , _return_type{return_type}
    , _arg_types{std::move(arg_types)}
  {}

  size_t size() const override;
  size_t align() const override;
  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const FunctionType &other) const;
  const Type *return_type() const { return _return_type; }
  size_t arg_types_count() const { return _arg_types.size(); }
  const Type *arg_type(size_t n) const { return _arg_types[n]; }
};

struct StructField {
  std::string name;
  Type *type;
};

struct StructType final : Type {
private:
  std::vector<StructField> _fields;
  std::unordered_map<std::string_view, size_t> _field_names;
  size_t _size;
  size_t _align;

public:
  explicit StructType(std::string name, std::vector<StructField> fields);

  size_t size() const override;
  size_t align() const override;
  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const StructType &other) const;
  size_t fields_count() const { return _fields.size(); }
  const StructField &field(size_t n) const { return _fields[n]; }
  const StructField *field(std::string_view name) {
    auto it = _field_names.find(name);
    if (it == _field_names.end()) {
      return nullptr;
    }
    return &_fields[it->second];
  }
};

} // namespace lava::syn

namespace std {
  template<> struct hash<lava::syn::Type> {
    size_t operator()(const lava::syn::Type &type) const {
      return type.hash();
    }
  };
}

#endif // LAVA_SYN_TYPES_H_
