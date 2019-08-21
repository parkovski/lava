#ifndef ASH_DOCUMENT_CURSOR_H_
#define ASH_DOCUMENT_CURSOR_H_

#include "../ash.h"

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
  using lc_size_type = unsigned;
  // Offset type for line and column.
  using lc_offset_type = int;
  // 1-based (line, column) position.
  using position_type = std::pair<lc_size_type, lc_size_type>;
  // 0-based (line, column) offset. Line and column are still 1-based; these
  // offsets are relative to 0 as an offset is.
  using position_offset_type = std::pair<lc_offset_type, lc_offset_type>;

#ifdef __cpp_char8_t
  using char_type = char8_t;
#else
  using char_type = char;
#endif

  // UTF-32 codepoint type, useful when dealing with a single character.
  using codepoint_type = char32_t;

  // Invalid position.
  constexpr static size_type npos = size_type(-1);

  // Invalid character/codepoint.
  constexpr static codepoint_type nchar = char32_t(-1);

  // }}} Types

  // Cursor movement {{{

  // Set the absolute position.
  virtual void moveTo(size_type index) = 0;

  // Move the cursor by an offset.
  virtual Cursor &operator+=(offset_type offset) = 0;

  virtual Cursor &operator+=(position_offset_type offset) = 0;

  // Move the cursor backward by an offset.
  Cursor &operator-=(offset_type offset)
  { return *this += -offset; }

  Cursor &operator-=(position_offset_type offset)
  { return *this += std::make_pair(-offset.first, -offset.second); }

  // Move the cursor forward by 1.
  Cursor &operator++()
  { return *this += 1; }

  // Move the cursor backward by 1.
  Cursor &operator--()
  { return *this -= 1; }

  // }}} Cursor movement

  // Text accessors {{{

  // Document length in UTF-8 characters/Unicode code points.
  virtual size_type length() const = 0;

  // Document size in bytes.
  virtual size_type size() const = 0;

  // Returns the byte size of a substring of the document.
  virtual size_type size(size_type count) const = 0;

  // Number of lines in the document.
  virtual size_type lines() const = 0;

  // Char index span for a line. Line index is 1-based.
  virtual std::pair<size_type, size_type> line(lc_size_type index) const;

  // Get the code point at the current index.
  virtual codepoint_type operator*() const = 0;

  // Get the code point at an offset to the current index.
  virtual codepoint_type operator[](offset_type offset) const = 0;

  // Try to copy a substring of length count from the document into buf. Only
  // copies up to bufsize bytes. Returns the number of characters copied, with
  // bufsize set to the number of bytes copied.
  virtual size_type substr(char_type *buf, size_type *bufsize, size_type count)
                           const = 0;

  // Read a substring from the document into str.
  //virtual size_type substr(std::string &str, size_type count) const = 0;

  // }}} Text accessors

  // Cursor information {{{

  // Get the current position of the cursor.
  virtual size_type index() const = 0;

  // Get the 1-based line and column of the cursor.
  virtual position_type position() const = 0;

  // Convert a (line, column) pair to an index.
  virtual size_type positionToIndex(position_type position) const = 0;

  virtual position_type indexToPosition(size_type index) const = 0;

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
