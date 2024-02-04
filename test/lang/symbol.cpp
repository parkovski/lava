#include <catch2/catch_test_macros.hpp>
#include "lava/lang/symbol.h"

using namespace lava::lang;

TEST_CASE("Symbol table init", "[symbol]") {
  PointerType::TargetPointerSize = sizeof(size_t);
  SymbolTable symtab;
  auto const &int32_type = symtab.int_type(4, true);
  REQUIRE(int32_type.size == 4);
  REQUIRE(int32_type.is_signed == true);
  auto *int32_alias = symtab.global_namespace().get(symtab.intern("int32"));
  REQUIRE(*static_cast<TypeAlias*>(int32_alias)->type == int32_type);
}

TEST_CASE("Array type cache", "[symbol]") {
  PointerType::TargetPointerSize = sizeof(size_t);
  SymbolTable symtab;
  auto const &int32_type = symtab.int_type(4, true);
  auto const &array_type_1 = symtab.array_type(ArrayType{
    &int32_type,
    4
  });
  auto const &array_type_2 = symtab.array_type(ArrayType{
    &int32_type,
    4
  });

  REQUIRE(array_type_1 == array_type_2);
  REQUIRE(&array_type_1 == &array_type_2);
}

TEST_CASE("Tuple type cache", "[symbol]") {
  PointerType::TargetPointerSize = sizeof(size_t);
  SymbolTable symtab;
  StructType::FieldVector fields1;
  StructType::FieldVector fields2;

  fields1.emplace_back(&symtab.int_type(1, true));
  fields1.emplace_back(&symtab.int_type(2, true));
  fields1.emplace_back(&symtab.float_type(4));

  fields2.emplace_back(&symtab.int_type(1, true));
  fields2.emplace_back(&symtab.int_type(2, true));
  fields2.emplace_back(&symtab.float_type(4));

  auto const &tuple_type_1 = symtab.struct_type(StructType{fields1});
  auto const &tuple_type_2 = symtab.struct_type(StructType{fields2});
  REQUIRE(tuple_type_1 == tuple_type_2);
  REQUIRE(&tuple_type_1 == &tuple_type_2);
  REQUIRE(tuple_type_1.size == 8);
  REQUIRE(tuple_type_1.align == 4);
  REQUIRE(tuple_type_1.fields[0].offset == 0);
  REQUIRE(tuple_type_1.fields[1].offset == 2);
  REQUIRE(tuple_type_1.fields[2].offset == 4);
}

TEST_CASE("Struct type cache", "[symbol]") {
  PointerType::TargetPointerSize = sizeof(size_t);
  SymbolTable symtab;
  StructType::FieldVector fields1;
  StructType::FieldVector fields2;

  fields1.emplace_back(symtab.intern("first"), &symtab.int_type(4, true));
  fields1.emplace_back(symtab.intern("second"), &symtab.int_type(8, true));
  fields2.emplace_back(symtab.intern("first"), &symtab.int_type(4, true));
  fields2.emplace_back(symtab.intern("second"), &symtab.int_type(8, true));

  auto const &struct_type_1 = symtab.struct_type(fields1);
  auto const &struct_type_2 = symtab.struct_type(fields2);
  REQUIRE(struct_type_1 == struct_type_2);
  REQUIRE(&struct_type_1 == &struct_type_2);
  REQUIRE(struct_type_1.fields[0].offset == 0);
  REQUIRE(struct_type_1.fields[1].offset == 8);
  REQUIRE(struct_type_1.size == 16);
  REQUIRE(struct_type_1.align == 8);
}

TEST_CASE("Function type cache", "[symbol]") {
  PointerType::TargetPointerSize = sizeof(size_t);
  SymbolTable symtab;

  FunctionType::ArgVector args1;
  FunctionType::ArgVector args2;

  args1.emplace_back(symtab.intern("first"), &symtab.int_type(false));
  args1.emplace_back(symtab.intern("second"), &symtab.int_type(8, false));

  args2.emplace_back(symtab.intern("first"), &symtab.int_type(false));
  args2.emplace_back(symtab.intern("second"), &symtab.int_type(8, false));

  auto const &fnty1 = symtab.function_type(FunctionType{
    &symtab.int_type(false),
    std::move(args1)
  });
  auto const &fnty2 = symtab.function_type(FunctionType{
    &symtab.int_type(false),
    std::move(args2)
  });
  REQUIRE(fnty1 == fnty2);
  REQUIRE(&fnty1 == &fnty2);
}
