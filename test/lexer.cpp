#include "ash/parser/lexer.h"

#include <catch2/catch.hpp>

#include <string_view>

#ifdef ASH_LEX_TEST_OUT
# include <fstream>
#else
# include <sstream>
#endif

static const std::string_view program = R"~~~(
#!/usr/bin/env ash
<# This is a block comment
 # It spans multiple lines. #>
echo 'Hello world'
let int = 123456
let hex = 0x123abc
let bin = 0b001100
let flt = 1., .1, 1.0, 1.e2, .0e-1, 1.5e+5
let s = "int: $int, hex+bin: $(hex + bin)"
echo "\e[36mcolors\e[m"
echo $0 $# $? ${foo bar}
int += 1; hex **= 2
midi::listen {
  echo $1.name
}
)~~~";

static const std::string_view expected_output =
R"~~~(1:1: NewLine
2:1: CommentLine('#')
2:2: Text('!/usr/bin/env ash')
2:19: NewLine
3:1: CommentBlockStart('<#')
3:3: Text(' This is a block comment
 # It spans multiple lines. ')
4:29: CommentBlockEnd('#>')
4:31: NewLine
5:1: Ident('echo')
5:5: Whitespace
5:6: SingleQuote(''')
5:7: Text('Hello world')
5:18: SingleQuote(''')
5:19: NewLine
6:1: Ident('let')
6:4: Whitespace
6:5: Ident('int')
6:8: Whitespace
6:9: Equal('=')
6:10: Whitespace
6:11: IntLit('123456')
6:17: NewLine
7:1: Ident('let')
7:4: Whitespace
7:5: Ident('hex')
7:8: Whitespace
7:9: Equal('=')
7:10: Whitespace
7:11: HexLit('0x123abc')
7:19: NewLine
8:1: Ident('let')
8:4: Whitespace
8:5: Ident('bin')
8:8: Whitespace
8:9: Equal('=')
8:10: Whitespace
8:11: BinLit('0b001100')
8:19: NewLine
9:1: Ident('let')
9:4: Whitespace
9:5: Ident('flt')
9:8: Whitespace
9:9: Equal('=')
9:10: Whitespace
9:11: FloatLit('1.')
9:13: Comma(',')
9:14: Whitespace
9:15: FloatLit('.1')
9:17: Comma(',')
9:18: Whitespace
9:19: FloatLit('1.0')
9:22: Comma(',')
9:23: Whitespace
9:24: FloatLit('1.e2')
9:28: Comma(',')
9:29: Whitespace
9:30: FloatLit('.0e-1')
9:35: Comma(',')
9:36: Whitespace
9:37: FloatLit('1.5e+5')
9:43: NewLine
10:1: Ident('let')
10:4: Whitespace
10:5: Ident('s')
10:6: Whitespace
10:7: Equal('=')
10:8: Whitespace
10:9: DoubleQuote('"')
10:10: Text('int: ')
10:15: Variable('$int')
10:19: Text(', hex+bin: ')
10:30: DollarLeftParen('$(')
10:32: Ident('hex')
10:35: Whitespace
10:36: Plus('+')
10:37: Whitespace
10:38: Ident('bin')
10:41: LeftParen(')')
10:42: DoubleQuote('"')
10:43: NewLine
11:1: Ident('echo')
11:5: Whitespace
11:6: DoubleQuote('"')
11:7: Escape('\e')
11:9: Text('[36mcolors')
11:19: Escape('\e')
11:21: Text('[m')
11:23: DoubleQuote('"')
11:24: NewLine
12:1: Ident('echo')
12:5: Whitespace
12:6: Variable('$0')
12:8: Whitespace
12:9: Variable('$#')
12:11: Whitespace
12:12: Variable('$?')
12:14: Whitespace
12:15: DollarLeftBrace('${')
12:17: Ident('foo')
12:20: Whitespace
12:21: Ident('bar')
12:24: LeftBrace('}')
12:25: NewLine
13:1: Ident('int')
13:4: Whitespace
13:5: PlusEqual('+=')
13:7: Whitespace
13:8: IntLit('1')
13:9: Semicolon(';')
13:10: Whitespace
13:11: Ident('hex')
13:14: Whitespace
13:15: StarStarEqual('**=')
13:18: Whitespace
13:19: IntLit('2')
13:20: NewLine
14:1: Ident('midi')
14:5: ColonColon('::')
14:7: Ident('listen')
14:13: Whitespace
14:14: LeftBrace('{')
14:15: NewLine
15:1: Whitespace
15:3: Ident('echo')
15:7: Whitespace
15:8: Variable('$1')
15:10: Dot('.')
15:11: Ident('name')
15:15: NewLine
16:1: RightBrace('}')
16:2: NewLine
)~~~";

TEST_CASE("Lexer", "[lexer]") {
  using namespace ash::parser;
  using namespace ash::source;

  SourceLocator locator;
  Session session(&locator, locator.addFile("test", program));
  Lexer lexer(&session);
#ifdef ASH_LEX_TEST_OUT
  std::ofstream out("lex.test.out");
#else
  std::stringstream out;
#endif
  while (true) {
    Token t = lexer();
    if (t.id == Tk::EndOfInput) {
      break;
    }
    auto startLoc = locator.find(t.loc);
    auto endLoc = locator.findNext(t.loc);
    out << startLoc.line << ":" << startLoc.column << ": " << t.id;
    switch (t.id) {
      case Tk::NewLine:
      case Tk::Whitespace:
      case Tk::Invalid:
        break;
      default:
        out
          << "('"
          << program.substr(startLoc.index, endLoc.index - startLoc.index)
          << "')";
    }
    out << "\n";
  }

#ifndef ASH_LEX_TEST_OUT
  REQUIRE(out.str() == expected_output);
#else
  INFO("Lexer output written to lex.test.out.");
#endif
}
