#ifndef LAVA_SYN_TYPES_H_
#define LAVA_SYN_TYPES_H_

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <string_view>

namespace lava::syn {

enum class TypeKind {
  Never,
  Void,
  Data,
  Int,
  Float,
  Vector,
  Pointer,
  Fun,
  Struct,
};

struct Type {
private:
  std::string _name;

public:
  explicit Type() noexcept
    : _name{}
  {}

  explicit Type(std::string name) noexcept
    : _name{std::move(name)}
  {}

  virtual ~Type() = 0;
  virtual TypeKind type_kind() const = 0;
  virtual size_t hash() const = 0;
  bool operator==(const Type &other) const;
  std::string_view get_name() const { return _name; }
};

struct NeverType final : Type {
  explicit NeverType() noexcept
    : Type{"never"}
  {}

  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const NeverType &other) const;
};

struct VoidType final : Type {
  explicit VoidType() noexcept
    : Type{"void"}
  {}

  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const VoidType &other) const;
};

struct DataType : Type {
  uint32_t size;
  uint32_t align;

  explicit DataType(std::string name, uint32_t size, uint32_t align) noexcept
    : Type{std::move(name)}
    , size{size}
    , align{align}
  {}

  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const DataType &other) const;
};

struct IntType final : DataType {
  bool is_signed;

  explicit IntType(std::string name, uint32_t size, bool is_signed) noexcept
    : DataType{std::move(name), size, size}
    , is_signed{is_signed}
  {}

  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const IntType &other) const;
};

struct FloatType final : DataType {
  explicit FloatType(std::string name, uint32_t size) noexcept
    : DataType{std::move(name), size, size}
  {}

  TypeKind type_kind() const override;
  bool operator==(const FloatType &other) const;
};

struct VectorType final : DataType {
  explicit VectorType(std::string name, uint32_t size) noexcept
    : DataType{std::move(name), size, size}
  {}

  TypeKind type_kind() const override;
  bool operator==(const VectorType &other) const;
};

struct PointerType final : DataType {
  Type *pointee;

  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const PointerType &other) const;
};

struct FunType final : Type {
  std::vector<Type*> args;
  Type *ret;

  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const FunType &other) const;
};

struct Field {
  std::string name;
  Type *type;
};

struct StructType final : DataType {
  std::vector<Field> fields;
  std::unordered_map<std::string_view, size_t> field_names;

  TypeKind type_kind() const override;
  size_t hash() const override;
  bool operator==(const StructType &other) const;
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
