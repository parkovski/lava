#ifndef ASH_AST_WRITER_H_
#define ASH_AST_WRITER_H_

#include <string>
#include <string_view>
#include <iosfwd>

namespace ash::source {
  class Session;
}

namespace ash::ast {

class Writer {
public:
  explicit Writer(const source::Session &session, std::ostream &os,
                  unsigned indentSize = 2) noexcept
    : _session{&session}, _os{&os}, _indentSize{indentSize}
  {}

  const source::Session &session() const {
    return *_session;
  }

  friend struct Indenter;
  struct Indenter {
    explicit Indenter(Writer &writer) noexcept : _writer{&writer} {
      for (unsigned i = 0; i < _writer->_indentSize; ++i) {
        _writer->_indent.push_back(' ');
      }
    }

    Indenter(const Indenter &) = delete;
    Indenter &operator=(const Indenter &) = delete;

    ~Indenter() {
      _writer->_indent.erase(_writer->_indent.length() - _writer->_indentSize);
    }

  private:
    Writer *_writer;
  };

  Indenter indented() {
    return Indenter(*this);
  }

  std::string_view indent() const {
    return _indent;
  }

  template<typename T>
  Writer &operator<<(T &&item) {
    (*_os) << std::forward<T>(item);
    return *this;
  }

private:
  const source::Session *_session;
  std::ostream *_os;
  unsigned _indentSize;
  std::string _indent;
};

} // namespace ash::ast

#endif // ASH_AST_WRITER_H_
