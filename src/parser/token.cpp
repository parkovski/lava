#include "ash/parser/token.h"

#include <ostream>

namespace ash::parser {

  std::string_view to_string(Tk tk) {
#define ASH_TK(t) case Tk::t: return #t;
    switch (tk) {
      default: return {};
      ASH_TOKENS(ASH_TK)
    }
#undef ASH_TK
  }

  std::ostream &operator<<(std::ostream &os, Tk tk) {
    return os << to_string(tk);
  }

} // namespace ash::parser
