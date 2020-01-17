#ifndef ASH_DOCUMENT_CURSOR_H_
#define ASH_DOCUMENT_CURSOR_H_

#include "../ash.h"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace ash::doc {

// Note: char_type becomes char8_t in C++20, use u8"" literals.
class Cursor {
public:
  // Types {{{

  // Size type for char or byte index.
  using size_type = size_t;
  // Offset type for char or byte index.
  using offset_type = ptrdiff_t;

  // Size type for line and column.
  using grid_size_type = uint32_t;
  // 1-based (line, column) position.
  using position_type = std::pair<grid_size_type, grid_size_type>;

#ifdef __cpp_char8_t
  using char_type = char8_t;
#else
  using char_type = char;
#endif

  // Invalid position.
  constexpr static size_type npos = size_type(-1);

  // Invalid character/codepoint.
  constexpr static char32_t nchar = char32_t(-1);

  // }}} Types

  // Cursor movement {{{

  // Set the absolute position.
  virtual void moveTo(size_type index) = 0;
  // Set the relative position.
  virtual void moveBy(offset_type offset) = 0;

  // Move the cursor by an offset.
  Cursor &operator+=(offset_type offset) {
    moveBy(offset); return *this;
  }
  // Move the cursor backward by an offset.
  Cursor &operator-=(offset_type offset) {
    moveBy(-offset); return *this;
  }
  // Move the cursor forward by 1.
  Cursor &operator++() {
    moveBy(1); return *this;
  }
  // Move the cursor backward by 1.
  Cursor &operator--() {
    moveBy(-1); return *this;
  }

  // }}} Cursor movement

  // Text accessors {{{

  // Get the code point at an offset to the current index.
  virtual char32_t operator[](offset_type offset) const = 0;
  // Get the code point at the current index.
  virtual char32_t operator*() const = 0;
  // Try to copy a substring of length count from the document into buf. Only
  // copies up to bufsize bytes. Returns the number of characters copied, with
  // bufsize set to the number of bytes copied.
  virtual size_type substr(char_type *buf, size_type *bufsize, size_type count)
                           const = 0;
  // Read a substring from the document into str.
  //virtual size_type substr(std::string &str, size_type count) const = 0;

  // }}} Text accessors

  // Cursor information {{{

  // Minimum valid offset relative to cursor position.
  virtual offset_type minOffset() const = 0;
  // Maximum valid offset relative to cursor position.
  virtual offset_type maxOffset() const = 0;
  // Get the current position of the cursor.
  virtual size_type index() const = 0;
  // Get the 1-based line and column of the cursor.
  virtual position_type position() const = 0;

  // }}} Cursor information

  // Document modification {{{

  // Insert text at the current position.
  virtual size_type insert(const char_type *text) = 0;
  // Replace text at the current position.
  virtual size_type replace(size_type count, const char_type *text) = 0;
  // Erase text at the current position.
  virtual void erase(size_type count) = 0;
  // Clear the whole document.
  virtual void clear() = 0;

  // }}} Document modification
};

} // namespace ash::doc

#endif // ASH_DOCUMENT_CURSOR_H_
