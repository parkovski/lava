#include "ash/ast/node.h"
#include "ash/source/session.h"

#include <ostream>

using namespace ash::ast;
using namespace ash::source;

Node::~Node() = default;

Trivia::~Trivia() = default;

SpanRef Trivia::span() const {
  return _span;
}

void Trivia::write(Writer &) const {
}

NodeList::~NodeList() = default;

SpanRef NodeList::span() const {
  if (_nodes.empty()) {
    return {};
  }

  return {_nodes.front().span().first, _nodes.back().span().second};
}

void NodeList::write(Writer &w) const {
  if (_nodes.empty()) {
    w << w.indent() << "[]";
    return;
  }

  w << "[";
  for (size_t i = 0; i < _nodes.size() - 1; ++i) {
    _nodes[i].write(w);
    w << ", ";
  }
  _nodes.back().write(w);
  w << "]";
}

Terminal::~Terminal() = default;

SpanRef Terminal::span() const {
  return {_location, _location};
}

void Terminal::write(Writer &w) const {
  w << w.session().locator().text(_location);
}

Separator::~Separator() = default;

SpanRef Separator::span() const {
  if (_separators.empty()) {
    return {};
  }
  return {_separators.front().span().first, _separators.back().span().second};
}

void Separator::write(Writer &w) const {
  w << "SEP*" << _separators.size();
}

SeparatedNodeList::~SeparatedNodeList() = default;

SpanRef SeparatedNodeList::span() const {
  if (_lists.empty()) {
    if (_firstSeparator.empty()) {
      return {};
    }
    return _firstSeparator.span();
  }

  auto const &last = _lists.back();
  if (!_firstSeparator.empty()) {
    if (last.second.empty()) {
      return {_firstSeparator.span().first, last.first.span().second};
    } else {
      return {_firstSeparator.span().first, last.second.span().second};
    }
  } else if (last.second.empty()) {
    return last.first.span();
  } else {
    return {last.first.span().first, last.second.span().second};
  }
}

void SeparatedNodeList::write(Writer &w) const {
  w << w.indent() << "[\n";
  {
    auto indented = w.indented();
    if (!_firstSeparator.empty()) {
      w << w.indent();
      _firstSeparator.write(w);
      if (!_lists.empty()) {
        w << ",\n";
      }
    }

    if (!_lists.empty()) {
      for (size_t i = 0; i < _lists.size() - 1; ++i) {
        w << w.indent();
        _lists[i].first.write(w);
        w << ",\n";
        if (!_lists[i].second.empty()) {
          w << w.indent();
          _lists[i].second.write(w);
          w << ",\n";
        }
      }
      w << w.indent();
      _lists.back().first.write(w);
      if (!_lists.back().second.empty()) {
        w << ",\n" << w.indent();
        _lists.back().second.write(w);
      }
    }
  }
  w << '\n' << w.indent() << ']';
}

Literal::~Literal() = default;

StringLiteral::~StringLiteral() = default;

SpanRef StringLiteral::span() const {
  return {Terminal::span().first, _endLoc};
}

void StringLiteral::write(Writer &w) const {
  auto loc = Terminal::span().first;
  w << '\'';
  while (loc.isValid() && loc != _endLoc) {
    loc = w.session().locator().next(loc);
  }
  w << '\'';
}

Identifier::~Identifier() = default;

Variable::~Variable() = default;

Operator::~Operator() = default;

void Operator::write(Writer &w) const {
  Terminal::write(w);
}

String::~String() = default;

SpanRef String::span() const {
  return {_openQuote, _closeQuote};
}

void String::write(Writer &w) const {
  auto const &locator = w.session().locator();
  w << locator.text(_openQuote);
  for (auto const &piece : _pieces) {
    piece.write(w);
  }
  w << locator.text(_closeQuote);
}

Unary::~Unary() = default;

SpanRef Unary::span() const {
  return {_op.span().first, _expr->span().second};
}

void Unary::write(Writer &w) const {
  auto const &locator = w.session().locator();
  w << '(';
  if (locator.find(_op.span().first).index <
      locator.find(_expr->span().first).index) {
    // Prefix
    _op.write(w);
    w << ' ';
    _expr->write(w);
  } else {
    // Postfix
    _expr->write(w);
    w << ' ';
    _op.write(w);
  }
  w << ')';
}

Binary::~Binary() = default;

SpanRef Binary::span() const {
  return {_left->span().first, _right->span().second};
}

void Binary::write(Writer &w) const {
  w << '(';
  _left->write(w);
  w << ' ';
  _op.write(w);
  w << ' ';
  _right->write(w);
  w << ')';
}

Delimited::~Delimited() = default;

SpanRef Delimited::span() const {
  return {_openDelim.span().first, _closeDelim.span().second};
}

void Delimited::write(Writer &w) const {
  w << "Delim";
  _openDelim.write(w);
  if (!_exprs.empty()) {
    for (size_t i = 0; i < _exprs.size() - 1; ++i) {
      _exprs[i].write(w);
      w << "; ";
    }
    _exprs.back().write(w);
  }
  _closeDelim.write(w);
}
