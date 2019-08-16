#ifndef ASH_DOCUMENT_CURSOR_H_
#define ASH_DOCUMENT_CURSOR_H_

#include "ash/ash.h"

namespace ash::doc {

class Cursor {
public:
  // Cursor movement {{{

  // Set the absolute position.
  virtual void set(size_t position) = 0;

  // Move the cursor by an offset.
  virtual Cursor &operator+=(ptrdiff_t count) = 0;

  // Move the cursor backward by an offset.
  Cursor &operator-=(ptrdiff_t count)
  { return *this += -count; }

  // Move the cursor forward by 1.
  Cursor &operator++()
  { return *this += 1; }

  // Move the cursor backward by 1.
  Cursor &operator--()
  { return *this -= 1; }

  // }}} Cursor movement

  // Text accessors {{{

  // Document length in characters.
  virtual size_t length() const = 0;

  // Get the character at the current index.
  virtual char32_t operator*() const = 0;

  // Get the character at an offset to the current index.
  virtual char32_t operator[](ptrdiff_t offset) const = 0;

  // Read a substring from the document into buf.
  virtual size_t substr(char *buf, size_t *bufsize, size_t count) = 0;

  // }}} Text accessors

  // Cursor information {{{

  // Get the current position of the cursor.
  virtual size_t position() const = 0;

  // Convert a (line, column) pair to an index.
  virtual size_t to_position(size_t line, size_t column) const = 0;

  // Get the current line of the cursor.
  virtual size_t line() const = 0;

  // Get the current column of the cursor.
  virtual size_t column() const = 0;

  // }}} Cursor information

  // Document modification {{{

  // Insert text at the current position.
  virtual size_t insert(const char *text) = 0;

  // Replace text at the current position.
  virtual size_t replace(size_t count, const char *text) = 0;

  // Erase text at the current position.
  virtual void erase(size_t count) = 0;

  // Clear the whole document.
  virtual void clear() = 0;

  // }}} Document modification
};

} // namespace ash::doc

#endif // ASH_DOCUMENT_CURSOR_H_
