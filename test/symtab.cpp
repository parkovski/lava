#include <catch2/catch_test_macros.hpp>
#include "ash/sym/symtab.h"
#include "ash/sym/attr.h"

using namespace ash::sym;
using namespace ash::sym::attr;

TEST_CASE("Symbol table", "[symtab]") {
  SymbolTable st;

  id_t id_root = st.next_id();
  id_t id_void = st.next_id();
  id_t id_int = st.next_id();

  auto cstr_root = "";
  auto cstr_void = "void";
  auto cstr_int = "int";

  std::string_view root_name{cstr_root};
  std::string_view void_name{cstr_void};
  std::string_view int_name{cstr_int};

  REQUIRE(st.find_str(root_name, true));
  REQUIRE(root_name.data() != cstr_root);

  REQUIRE(st.find_str(void_name, true));
  REQUIRE(void_name.data() != cstr_void);

  REQUIRE(st.find_str(int_name, true));
  REQUIRE(int_name.data() != cstr_int);

  auto *ns = st.cxxnew<Namespace>();
  REQUIRE(ns->set_name(void_name, id_void));
  REQUIRE(ns->set_name(int_name, id_int));
  REQUIRE(!ns->set_name(int_name, id_void));
  st.put_attr<Namespace>(id_root);
}
