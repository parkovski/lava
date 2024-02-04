#include <string_view>
#include "lava/lang/token.h"

std::string_view lava::lang::get_token_name(int what) {
  switch (what) {
  default: return "Invalid";
#define X(Name) case Tk##Name: return #Name;
  LAVA_TOKENS
#undef X
  }
}
