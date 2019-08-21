#ifndef ASH_DOCUMENT_H_
#define ASH_DOCUMENT_H_

#include "ash.h"

#include "collections/intervaltree.h"
#include "collections/slidingorderedset.h"

#include <cassert>
#include <cstddef>
#include <utility>

namespace ash::doc {

namespace detail {

/// Skip to the given UTF-8 offset in the string.
/// \param str The string to search.
/// \param chars The number of characters to skip.
/// \param bytelen The length of the string in bytes.
/// \return A pointer to the new offset in the string.
static inline u8char *skip_utf8(u8char *str, size_t chars, size_t bytelen) {
  size_t chars_seen = 0;
  const u8char *const end = str + bytelen;
  while (chars_seen < chars) {
    auto cp_size = utf8_codepoint_size(*str);
    if (str + cp_size > end) {
      break;
    }
    str += cp_size;
    ++chars_seen;
  }
  return str;
}

/// Copy a number of UTF-8 characters into a new buffer. Does not write a null.
/// \param dst The buffer to copy to.
/// \param src The buffer to copy from.
/// \param chars In: the number of characters to copy.
///              Out: the number of characters written.
/// \param max_size The maximum number of bytes to copy.
/// \return The number of bytes written.
static inline size_t copy_utf8(u8char *dst, const u8char *src, size_t *chars,
                               size_t max_size)
{
  size_t chars_seen = 0;
  const u8char *const start = src;
  const u8char *const end = src + max_size;
  const u8char *const safe_end = end - 6;

  while (src < safe_end && chars_seen < *chars) {
    switch (utf8_codepoint_size(*src)) {
      case 6: *dst++ = *src++;
      case 5: *dst++ = *src++;
      case 4: *dst++ = *src++;
      case 3: *dst++ = *src++;
      case 2: *dst++ = *src++;
      default: *dst++ = *src++;
    }
    ++chars_seen;
  }

  while (src < end && chars_seen < *chars) {
    auto cp_size = utf8_codepoint_size(*src);
    if (src + cp_size > end) {
      // Not enough room to write this character.
      break;
    }
    switch (cp_size) {
      case 6: *dst++ = *src++;
      case 5: *dst++ = *src++;
      case 4: *dst++ = *src++;
      case 3: *dst++ = *src++;
      case 2: *dst++ = *src++;
      default: *dst++ = *src++;
    }
    ++chars_seen;
  }

  *chars = chars_seen;
  return src - start;
}

} // namespace detail

// UTF-8 rope. Try to look like an std::string as much as possible.
// TODO: Iterators
class Rope final {
public:
  using char_type = u8char;
  constexpr static size_t npos = (size_t)-1;

  /// Construct an empty document.
  explicit Rope();

  /// Construct a document pre-populated with text.
  /// \param text The text to fill the document with.
  explicit Rope(const char_type *text);

  Rope(const Rope &);
  Rope &operator=(const Rope &);
  Rope(Rope &&) noexcept;
  Rope &operator=(Rope &&) noexcept;

  ~Rope();

  /// Insert text into the document.
  /// \param index UTF-8 character position.
  /// \param text UTF-8 string to insert.
  bool insert(size_t index, const char_type *text);

  // Length in UTF-8 characters.
  size_t length() const;

  // Size in bytes.
  size_t size() const;

  /// Append to the end of the document.
  /// \param text The text to append.
  bool append(const char_type *text);

  /// Delete a range of characters from the document. Indexes in characters.
  /// \param index The first character to delete.
  /// \param count The number of characters to delete.
  void erase(size_t index, size_t count);

  /// Replace a range with different text. Indexes in characters.
  /// \param index The first character to erase.
  /// \param count The number of characters to erase.
  /// \param text The new text to insert at \c index.
  bool replace(size_t index, size_t count, const char_type *text);

  // Clears all text and attributes.
  void clear();

  /// Read a range of text from the rope into a buffer. Indexes in characters.
  /// Does _not_ append a null to the buffer.
  /// \param buf The buffer to receive the text.
  /// \param bufsize The size of the buffer in bytes. On return, this will be
  ///                set to the number of bytes written to the buffer.
  /// \param index The first character to read.
  /// \param count The maximum number of characters to read.
  /// \return The number of characters written to the buffer.
  size_t substr(char_type *buf, size_t *bufsize, size_t index, size_t count)
                const;

  /// Reads a substring from the rope into a buffer, including a NUL ('\0')
  /// character at the end. Returns the number of UTF-8 characters written,
  /// not including the terminating NUL.
  /// \see substr.
  size_t subcstr(char_type *buf, size_t *bufsize, size_t index, size_t count)
                 const;

  /// Get one Unicode character from the document.
  /// Note: Works in log(N) time! Try to read more in chunks for better perf.
  /// \param index UTF-8 character offset.
  /// \return The UTF-32 character at the specified offset.
  char32_t operator[](size_t index) const;

private:
#ifdef ASH_DOCUMENT_IMPL
  rope *_text;
#else
  void *_text;
#endif
};

// Document requirements (all operations must be relatively quick):
// - Insert/remove text at arbitrary positions.
//   + librope
// - 1:1 mapping line number <-> character position.
//   + Quickly shift all positions greater than some offset.
//   + Quickly inc/dec all line numbers greater than some offset.
//   + Red-black tree deriving line number from subtree size and absolute
//     position from an offset of parent.
// - 1:N mapping text span -> arbitrary data.
//   + Spans may overlap in general.
//   + Different data types can specify span rules, ex:
//     * Lexer tokens cannot overlap.
//     * Parse trees cannot overlap but can fully contain each other.
//     * Diagnostic data has no overlap requirements but should align with
//       token boundaries.
// - Notify on change.

class Document {
public:
  using char_type = Rope::char_type;
  static constexpr size_t npos = Rope::npos;
  using lc_size_type = size_t;
  using lc_offset_type = ptrdiff_t;

  /// Construct an empty document.
  explicit Document();

  /// Construct a document pre-populated with text.
  /// \param text The text to fill the document with.
  explicit Document(const char_type *text);

  virtual ~Document();

protected:
  virtual void onInsert(size_t index, const char_type *text, size_t chars, size_t bytes);
  virtual void onAppend(const char_type *text, size_t chars, size_t bytes);
  virtual void onReplace(size_t index, size_t charsRemoved, const char_type *text, size_t charsInserted, ptrdiff_t deltaBytes);
  virtual void onErase(size_t index, size_t charsErased, size_t bytesErased);
  virtual void onClear(size_t chars, size_t bytes);

private:
  void markNewlines(size_t index, const char_type *text);

public:
  /// Insert text into the document.
  /// \param index UTF-8 character position.
  /// \param text UTF-8 string to insert.
  bool insert(size_t index, const char_type *text);

  // Length in UTF-8 characters.
  size_t length() const;

  // Size in bytes.
  size_t size() const;

  /// Append to the end of the document.
  /// \param text The text to append.
  bool append(const char_type *text);

  /// Replace a range with different text. Indexes in characters.
  /// \param index The first character to delete.
  /// \param count The number of characters to delete.
  /// \param text The new text to insert at \c from.
  bool replace(size_t index, size_t count, const char_type *text);

  /// Delete a range of characters from the document. Indexes in characters.
  /// \param index The first character to delete.
  /// \param count The number of characters to delete.
  void erase(size_t index, size_t count);

  // Clears all text and attributes.
  void clear();

  /// \see Rope::substr.
  size_t substr(char_type *buf, size_t *bufsize, size_t index, size_t count)
                const;

  /// \see Rope::subcstr.
  size_t subcstr(char_type *buf, size_t *bufsize, size_t index, size_t count)
                 const;

  /// Get one Unicode character from the document.
  /// Note: Works in log(N) time! Try to read more in chunks for better perf.
  /// \param index UTF-8 character offset.
  /// \return The UTF-32 character at the specified offset.
  char32_t operator[](size_t index) const;

  // The document always has at least one line.
  lc_size_type lines() const;

  // Returns a 1-based line index.
  lc_size_type lineAt(size_t index) const;

  // Make line and column a valid pair if either is out of range.
  void constrain(lc_size_type &line, lc_size_type &column) const;

  // Returns the (line, column) pair for a character position.
  // If index is out of range, the last character position is returned.
  std::pair<lc_size_type, lc_size_type> indexToPoint(size_t index) const;

  // Returns a character position for a (line, column) pair.
  // Out of range values are wrapped to [1, max].
  size_t pointToIndex(lc_size_type line, lc_size_type column) const;

  // Line numbers start at 1. The end of the span is the index of the newline
  // character, or for the last line it is the character length of the document.
  std::pair<size_t, size_t> spanForLine(lc_size_type line) const;

private:
  Rope _rope;
  collections::SlidingOrderedSet<> _newlines;
};

// This document can store attributes.
template<typename Attribute>
class CoolDocument : public Document {
public:
  using Document::Document;

protected:
  void onInsert(size_t index, const char_type *, size_t chars, size_t)
                override {
    _attrs.shift(index, chars);
  }

  void onAppend(const char_type *, size_t, size_t) override
  {}

  void onReplace(size_t index, size_t charsErased, const char_type *,
                 size_t charsInserted, ptrdiff_t) override {
    _attrs.shift(index, -static_cast<ptrdiff_t>(charsErased));
    _attrs.shift(index, charsInserted);
  }

  void onErase(size_t index, size_t charsErased, size_t) override {
    _attrs.shift(index, -static_cast<ptrdiff_t>(charsErased));
  }

  void onClear(size_t, size_t) override {
    _attrs.clear();
  }

  collections::IntervalTree<Attribute> &attrs() {
    return _attrs;
  }

  const collections::IntervalTree<Attribute> &attrs() const {
    return _attrs;
  }

private:
  collections::IntervalTree<Attribute> _attrs;
};

} // namespace ash::doc

#endif // ASH_DOCUMENT_H_
