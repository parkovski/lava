#ifndef LAVA_LANG_SYMBOL_H_
#define LAVA_LANG_SYMBOL_H_

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <cassert>
#include <boost/container_hash/hash.hpp>
#include <boost/container/small_vector.hpp>

#include "instr.h"

namespace lava::lang {

  struct InternString {
    size_t offset;
    size_t size;

    InternString() noexcept : offset{0}, size{0} {}
    InternString(size_t offset, size_t size) noexcept
      : offset{offset}, size{size}
    {}
    InternString(const InternString&) = default;
    InternString &operator=(const InternString&) = default;

    bool operator==(const InternString &other) const {
      return offset == other.offset && size == other.size;
    }

    operator bool() const { return size != 0; }

    size_t hash() const {
      size_t hash = 0;
      boost::hash_combine(hash, offset);
      boost::hash_combine(hash, size);
      return hash;
    }
  };

  using SymbolPath = boost::container::small_vector<InternString, 1>;

} // namespace lava::lang

namespace std {
  template<> struct hash<lava::lang::InternString> {
    size_t operator()(const lava::lang::InternString &str) const {
      return str.hash();
    }
  };

  template<> struct hash<lava::lang::SymbolPath> {
    size_t operator()(const lava::lang::SymbolPath &path) const {
      size_t hash = 0;
      for (auto const &str : path) {
        boost::hash_combine(hash, str.offset);
        boost::hash_combine(hash, str.size);
      }
      return hash;
    }
  };
} // namespace std

namespace boost {
  template<> struct hash<lava::lang::InternString> {
    size_t operator()(const lava::lang::InternString &str) const {
      return str.hash();
    }
  };
}

namespace lava::lang {

struct Symbol {
private:
  InternString _name;

public:
  Symbol(InternString name)
    : _name{name}
  {}
  Symbol(const Symbol &) = default;
  Symbol &operator=(const Symbol &) = default;
  Symbol(Symbol&&) = default;
  Symbol &operator=(Symbol&&) = default;

  virtual ~Symbol() = 0;

  InternString name() const { return _name; }
};

struct Namespace : Symbol {
private:
  Namespace *_parent;
  std::vector<Namespace*> _using;
  std::vector<std::unique_ptr<Symbol>> _symbols_ordered;
  std::unordered_map<InternString, Symbol*> _symbols;

  Namespace()
    : Symbol{InternString{}}
    , _parent{nullptr}
  {}

  friend struct SymbolTable;

public:
  Namespace(Namespace &parent)
    : Symbol{InternString{}}
    , _parent{&parent}
  {}

  Namespace(InternString name, Namespace &parent)
    : Symbol{name}
    , _parent{&parent}
  {}

  ~Namespace();

  size_t size() const {
    return _symbols_ordered.size();
  }

  Symbol *operator[](size_t index) {
    return _symbols_ordered[index].get();
  }

  const Symbol *operator[](size_t index) const {
    return _symbols_ordered[index].get();
  }

  void clear() {
    _using.clear();
    _symbols_ordered.clear();
    _symbols.clear();
  }

  bool has(InternString name) const {
    return _symbols.find(name) != _symbols.end();
  }

  // Returns the new symbol if succeeded, else null.
  Symbol *add(std::unique_ptr<Symbol> value) {
    auto [it, inserted] = _symbols.emplace(value->name(), value.get());
    if (inserted) {
      return _symbols_ordered.emplace_back(std::move(value)).get();
    }
    return nullptr;
  }

  void add_using(Namespace &ns) {
    _using.emplace_back(&ns);
  }

  Symbol *get(InternString name) {
    auto it = _symbols.find(name);
    if (it == _symbols.end()) {
      return nullptr;
    }
    return it->second;
  }

  const Symbol *get(InternString name) const {
    return const_cast<Namespace*>(this)->get(name);
  }

  Symbol *get(const SymbolPath &path);

  const Symbol *get(const SymbolPath &path) const {
    return const_cast<Namespace*>(this)->get(path);
  }

  Symbol *getrec(InternString name);

  const Symbol *getrec(InternString name) const {
    return const_cast<Namespace*>(this)->getrec(name);
  }

  Symbol *getrec(const SymbolPath &path);

  const Symbol *getrec(const SymbolPath &path) const {
    return const_cast<Namespace*>(this)->getrec(path);
  }
};

enum class TypeKind {
  Never,
  Any,
  Void,
  Bool,
  Int,
  Float,
  Pointer,
  NullPointer,
  Array,
  Struct,
  Function,
};

struct Type {
  virtual ~Type();
  virtual TypeKind kind() const = 0;
  virtual size_t hash() const = 0;
  bool operator==(const Type &other) const;
};

struct NeverType : Type {
  ~NeverType();
  TypeKind kind() const override;
  size_t hash() const override;
  bool operator==(const NeverType &) const { return true; }
};

struct AnyType : Type {
  ~AnyType();
  TypeKind kind() const override;
  size_t hash() const override;
  bool operator==(const AnyType &) const { return true; }
};

struct DataType : Type {
  unsigned size;
  unsigned align;

  DataType(unsigned size) : size{size}, align{size} {}
  DataType(unsigned size, unsigned align) : size{size}, align{align} {}
  DataType(const DataType &) = default;
  DataType &operator=(const DataType &) = default;
  DataType(DataType&&) = default;
  DataType &operator=(DataType&&) = default;
};

struct VoidType : DataType {
  VoidType() : DataType(0) {}
  ~VoidType();
  TypeKind kind() const override;
  size_t hash() const override;
  bool operator==(const VoidType &) const { return true; }
};

struct BoolType : DataType {
  BoolType() : DataType(1) {}
  ~BoolType();
  TypeKind kind() const override;
  size_t hash() const override;
  bool operator==(const BoolType &) const { return true; }
};

struct IntType : DataType {
  bool is_signed;

  IntType(unsigned size, bool is_signed)
    : DataType{size}
    , is_signed{is_signed}
  {}

  ~IntType();

  TypeKind kind() const override;
  size_t hash() const override;
  bool operator==(const IntType &other) const;
};

struct FloatType : DataType {
private:
  FloatType(unsigned size) : DataType{size} {}

public:
  ~FloatType();

  TypeKind kind() const override;
  size_t hash() const override;
  bool operator==(const FloatType &other) const;

  static FloatType float_type() { return FloatType(4); }
  static FloatType double_type() { return FloatType(8); }
};

struct PointerType : DataType {
  static unsigned TargetPointerSize;

  const Type *pointed_at;

  PointerType(const Type *pointed_at)
    : DataType{TargetPointerSize}
    , pointed_at{pointed_at}
  {
    assert(TargetPointerSize != 0);
  }

  PointerType(const PointerType&) = default;
  PointerType &operator=(const PointerType&) = default;

  ~PointerType();

  TypeKind kind() const override;
  size_t hash() const override;
  bool operator==(const PointerType &other) const;
};

struct NullPointerType : PointerType {
  NullPointerType() : PointerType(nullptr) {}

  ~NullPointerType();

  TypeKind kind() const override;
  bool operator==(const NullPointerType &) const { return true; }
};

struct ArrayType : DataType {
  const DataType *element_type;
  unsigned array_length;

  static unsigned aligned_size(unsigned eltsize, unsigned eltalign,
                               unsigned size);


  ArrayType(const DataType *element_type, unsigned array_length);

  ArrayType(ArrayType&&) = default;
  ArrayType &operator=(ArrayType&&) = default;

  ~ArrayType();

  TypeKind kind() const override;
  size_t hash() const override;
  bool operator==(const ArrayType &other) const;
};

struct StructField {
  InternString name;
  const DataType *type;
  unsigned offset;

  StructField(InternString name, const DataType *type)
    : name{name}
    , type{type}
    , offset{0}
  {}

  StructField(const DataType *type)
    : name{}
    , type{type}
    , offset{0}
  {}

  StructField(const StructField&) = default;
  StructField &operator=(const StructField&) = default;
};

struct StructType : DataType {
  typedef boost::container::small_vector<StructField, 2> FieldVector;

  FieldVector fields;

  static unsigned get_align(const FieldVector &field_types);
  static unsigned align_fields(FieldVector &field_types);

  StructType(FieldVector fields)
    : DataType{align_fields(fields), get_align(fields)}
    , fields{std::move(fields)}
  {}

  StructType(StructType&&) = default;
  StructType &operator=(StructType&&) = default;

  ~StructType();

  TypeKind kind() const override;
  size_t hash() const override;
  bool operator==(const StructType &other) const;
};

struct FunctionArg {
  InternString name;
  const DataType *type;

  FunctionArg(InternString name, const DataType *type)
    : name{name}
    , type{type}
  {}

  FunctionArg(const DataType *type)
    : name{}
    , type{type}
  {}

  FunctionArg(const FunctionArg&) = default;
  FunctionArg &operator=(const FunctionArg&) = default;
};

struct FunctionType : Type {
  typedef boost::container::small_vector<FunctionArg, 2> ArgVector;

  const DataType *return_type;
  ArgVector arg_types;

  FunctionType(const DataType *return_type, ArgVector arg_types)
    : return_type{return_type}
    , arg_types{std::move(arg_types)}
  {}

  FunctionType(FunctionType&&) = default;
  FunctionType &operator=(FunctionType&&) = default;

  ~FunctionType();

  TypeKind kind() const override;
  size_t hash() const override;
  bool operator==(const FunctionType &other) const;

  // Compares only argument types, not names.
  bool are_types_same(const FunctionType &other) const;
};

} // namespace lava::lang

namespace std {
  template<> struct hash<lava::lang::PointerType> {
    size_t operator()(const lava::lang::PointerType &t) const {
      return t.hash();
    }
  };
  template<> struct hash<lava::lang::ArrayType> {
    size_t operator()(const lava::lang::ArrayType &t) const {
      return t.hash();
    }
  };
  template<> struct hash<lava::lang::StructType> {
    size_t operator()(const lava::lang::StructType &t) const {
      return t.hash();
    }
  };
  template<> struct hash<lava::lang::FunctionType> {
    size_t operator()(const lava::lang::FunctionType &t) const {
      return t.hash();
    }
  };
}

namespace lava::lang {

struct TypeAlias : Symbol {
  const Type *type;
  bool is_newtype;

  TypeAlias(InternString name, const Type *type, bool is_newtype = false)
    : Symbol{name}
    , type{type}
    , is_newtype{is_newtype}
  {}

  TypeAlias(const TypeAlias&) = default;
  TypeAlias &operator=(const TypeAlias&) = default;
};

struct BasicBlock {
  std::vector<Instruction> instrs;
};

struct Function : Symbol {
private:
  const FunctionType *_type;
  Namespace _args_ns;
  Namespace _locals_ns;
  std::vector<BasicBlock> _bbs;
  unsigned _registers;

  void add_args();

public:
  Function(InternString name, const FunctionType *type, Namespace &current_ns);

  const FunctionType *type() const { return _type; }

  // To be used to set an equivalent type with only differering arg names.
  void set_type(const FunctionType *type);

  Namespace &args_namespace() { return _args_ns; }
  const Namespace &args_namespace() const { return _args_ns; }

  Namespace &locals_namespace() { return _locals_ns; }
  const Namespace &locals_namespace() const { return _locals_ns; }

  void push_basicblock(BasicBlock &&bb) {
    _bbs.emplace_back(std::move(bb));
  }

  unsigned next_register() {
    unsigned r = _registers;
    ++_registers;
    return r;
  }

  const std::vector<BasicBlock> &basicblocks() const { return _bbs; }
};

struct Variable : Symbol {
private:
  const DataType *_type;

public:
  Variable(InternString name, const DataType *type)
    : Symbol{name}
    , _type{type}
  {}

  const DataType *type() const { return _type; }
};

struct SymbolTable {
private:
  struct string_hash {
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    size_t operator()(const char* str) const        { return hash_type{}(str); }
    size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    size_t operator()(std::string const& str) const { return hash_type{}(str); }
  };

  std::string _the_string;
  std::unordered_map<std::string, InternString, string_hash, std::equal_to<>>
    _string_map;
  size_t _anon_index;
  Namespace _global_ns;

  NeverType _never_type;
  AnyType _any_type;
  VoidType _void_type;
  BoolType _bool_type;
  NullPointerType _null_pointer_type;
  IntType _int8_type;
  IntType _uint8_type;
  IntType _int16_type;
  IntType _uint16_type;
  IntType _int32_type;
  IntType _uint32_type;
  IntType _int64_type;
  IntType _uint64_type;
  IntType _int128_type;
  IntType _uint128_type;
  IntType _int256_type;
  IntType _uint256_type;
  FloatType _float_type;
  FloatType _double_type;

  mutable std::unordered_set<PointerType> _pointer_types;
  mutable std::unordered_set<ArrayType> _array_types;
  mutable std::unordered_set<StructType> _struct_types;
  mutable std::unordered_set<FunctionType> _function_types;

  void add_base_types();

public:
  SymbolTable();

  InternString intern(std::string_view str);
  std::string_view get_string(InternString str);
  InternString get_anon_name();

  Namespace &global_namespace() { return _global_ns; }
  const Namespace &global_namespace() const { return _global_ns; }

  const NeverType &never_type() const { return _never_type; }
  const AnyType &any_type() const { return _any_type; }
  const VoidType &void_type() const { return _void_type; }
  const BoolType &bool_type() const { return _bool_type; }
  const NullPointerType &null_pointer_type() const {
    return _null_pointer_type;
  }
  const IntType &int_type(unsigned size, bool is_signed) const;
  const IntType &int_type(bool is_signed) const;
  const FloatType &float_type(unsigned size) const;
  const PointerType &pointer_type(PointerType &&pointer_type) const;
  const ArrayType &array_type(ArrayType &&array_type) const;
  const StructType &struct_type(StructType &&struct_type) const;
  const FunctionType &function_type(FunctionType &&function_type) const;
};

} // namespace lava::lang

#endif /* LAVA_LANG_SYMBOL_H_ */
