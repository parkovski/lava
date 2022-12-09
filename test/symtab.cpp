#include <catch2/catch_test_macros.hpp>

#include "lava/sym/symtab.h"

#include <sstream>
#include <cstring>

using namespace lava::sym;

TEST_CASE("Intern data", "[sym]") {
  Symtab st;
  std::stringstream ss;
  ss << "Hello";
  st.intern(ss.str());
  ss << " world!";
  st.intern(ss.str());

  auto ir = st.find_interned("Hello");
  REQUIRE((bool)ir);
  REQUIRE(!memcmp(ir.p, "Hello", ir.size));
  ir = st.find_interned("Hello world!");
  REQUIRE((bool)ir);
  REQUIRE(!memcmp(ir.p, "Hello world!", ir.size));

  size_t data[] = {
    1, 2, 3, 4
  };

  st.intern(data, alignof(size_t), sizeof(data));
  ir = st.find_interned(data, sizeof(data));
  REQUIRE(!memcmp(data, ir.p, sizeof(data)));
}

TEST_CASE("Symbols", "[sym]") {
  Symtab st;

  REQUIRE(st.get_own_attr_count(ID_root) > 0);
  REQUIRE(st.get_own_attr_at(ID_root, 0).first == ID_ScopeMap);
  REQUIRE(st.get_own_attr(ID_root, ID_ScopeMap));

  auto [id_number, inserted] = st.add_symbol(ID_root, "number");
  REQUIRE(inserted);
  REQUIRE(id_number != ID_undefined);

  // Test no duplicate insertion.
  uint32_t id_number_2;
  std::tie(id_number_2, inserted) = st.add_symbol(ID_root, "number");
  REQUIRE(!inserted);
  REQUIRE(id_number_2 == id_number);

  auto sm = st.add_attr<ScopeMap>(id_number, ID_ScopeMap);
  REQUIRE(sm);

  uint32_t id_int32, id_int64, id_float;

  std::tie(id_int32, inserted) = st.add_symbol(id_number, "int32");
  REQUIRE(inserted);
  REQUIRE((sm = st.add_attr<ScopeMap>(id_int32, ID_ScopeMap)));

  std::tie(id_int64, inserted) = st.add_symbol(id_number, "int64");
  REQUIRE(inserted);
  REQUIRE((sm = st.add_attr<ScopeMap>(id_int64, ID_ScopeMap)));

  std::tie(id_float, inserted) = st.add_symbol(id_number, "float");
  REQUIRE(inserted);
  REQUIRE((sm = st.add_attr<ScopeMap>(id_float, ID_ScopeMap)));

  InternRef ir_prototype = st.intern("prototype");
  uint32_t id_int32_proto, id_int64_proto, id_float_proto;
  std::tie(id_int32_proto, inserted) = st.add_symbol(id_int32, ir_prototype);
  REQUIRE(inserted);
  std::tie(id_int64_proto, inserted) = st.add_symbol(id_int64, ir_prototype);
  REQUIRE(inserted);
  std::tie(id_float_proto, inserted) = st.add_symbol(id_float, ir_prototype);
  REQUIRE(inserted);
}
