#ifndef ASH_DOCUMENT_DOCUMENT_H_
#define ASH_DOCUMENT_DOCUMENT_H_

#include "../collections/intervaltree.h"
#include "../collections/slidingorderedset.h"

#include "rope/rope.h"

#include <cassert>
#include <cstddef>
#include <utility>

namespace ash::doc {

namespace detail {
// Find out how many bytes the unicode character which starts with the specified byte
// will occupy in memory.
// Returns the number of bytes, or SIZE_MAX if the byte is invalid.
static inline size_t codepoint_size(uint8_t byte) {
  if (byte <= 0x7f) { return 1; } // 0x74 = 0111 1111
  else if (byte <= 0xbf) { return 1; } // 1011 1111. Invalid for a starting byte.
  else if (byte <= 0xdf) { return 2; } // 1101 1111
  else if (byte <= 0xef) { return 3; } // 1110 1111
  else if (byte <= 0xf7) { return 4; } // 1111 0111
  else if (byte <= 0xfb) { return 5; } // 1111 1011
  else if (byte <= 0xfd) { return 6; } // 1111 1101
  else { return 1; }
}

/// Skip to the given UTF-8 offset in the string.
/// \param str The string to search.
/// \param chars The number of characters to skip.
/// \param bytelen The length of the string in bytes.
/// \return A pointer to the new offset in the string.
static inline char *skip_utf8(char *str, size_t chars, size_t bytelen) {
  size_t chars_seen = 0;
  const char *const end = str + bytelen;
  while (chars_seen < chars) {
    auto cp_size = codepoint_size(*str);
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
static inline size_t copy_utf8(char *dst, const char *src, size_t *chars,
                               size_t max_size)
{
  size_t chars_seen = 0;
  const char *const start = src;
  const char *const end = src + max_size;
  const char *const safe_end = end - 6;

  while (src < safe_end && chars_seen < *chars) {
    switch (codepoint_size(*src)) {
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
    auto cp_size = codepoint_size(*src);
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

template<typename A>
class Document {
public:
  constexpr static const size_t npos = (size_t)-1;

  /// Construct an empty document.
  explicit Document() noexcept {
    _text = rope_new();
  }

  /// Construct a document pre-populated with text.
  /// \param text The text to fill the document with.
  explicit Document(const char *text) noexcept {
    _text = rope_new_with_utf8(reinterpret_cast<const uint8_t *>(text));
    mark_newlines(0, text);
  }

  ~Document() {
    rope_free(_text);
  }

private:
  void mark_newlines(size_t pos, const char *text) {
    size_t len = 0;
    size_t i = 0;
    char ch = text[0];
    while (ch) {
      if (ch == '\n') {
        _newlines.insert(pos + i);
      }
      ++len;
      i += detail::codepoint_size(ch);
      ch = text[i];
    }
  }

public:
  /// Insert text into the document.
  /// \param pos UTF-8 character position.
  /// \param text UTF-8 string to insert.
  void insert(size_t pos, const char *text) {
    auto old_len = char_length();
    if (rope_insert(_text, pos, reinterpret_cast<const uint8_t *>(text)) !=
        ROPE_OK) {
      return;
    }
    auto delta_len = char_length() - old_len;
    _newlines.shift(pos, delta_len);
    _attrs.shift(pos, delta_len);
    mark_newlines(pos, text);
  }

  /// Get the length of the text in UTF-8 characters.
  size_t char_length() const {
    return rope_char_count(_text);
  }

  /// Get the length of the text in bytes.
  size_t byte_length() const {
    return rope_byte_count(_text);
  }

  /// Append to the end of the document.
  /// \param text The text to append.
  void append(const char *text) {
    insert(this->char_length(), text);
  }

  /// Delete a range of characters from the document. Indexes in characters.
  /// \param from The beginning of the range to delete, inclusive.
  /// \param to The end of the range to delete, exclusive.
  void erase(size_t from, size_t to) {
    assert(to >= from);
    ptrdiff_t delta_len = static_cast<ptrdiff_t>(from) - to;
    _newlines.shift(from, delta_len);
    _attrs.shift(from, delta_len);
    rope_del(_text, from, -delta_len);
  }

  /// Replace a range with different text. Indexes in characters.
  /// \param from The beginning of the range to delete, inclusive.
  /// \param to The end of the range to delete, exclusive.
  /// \param text The new text to insert at \c from.
  void replace(size_t from, size_t to, const char *text) {
    erase(from, to);
    insert(from, text);
  }

  /// Read a range of text from the rope into a buffer. Indexes in characters.
  /// Does _not_ append a null to the buffer.
  /// \param buf The buffer to receive the text.
  /// \param bufsize The size of the buffer in bytes. On return, this will be
  ///                set to the number of bytes written to the buffer.
  /// \param from The beginning of the range to read from, inclusive.
  /// \param to The end of the range to read from, exclusive.
  /// \return The number of characters written to the buffer.
  size_t read(char *buf, size_t *bufsize, size_t from, size_t to) const {
    auto len = char_length();
    if (to > len) {
      to = len;
    }
    if (from >= to) {
      *bufsize = 0;
      return 0;
    }

    rope_node *node = &_text->head;
    size_t skipped = 0;

    // Find the starting node.
    while (skipped + node->nexts[0].skip_size < from) {
      // Use the skip list.
      auto height = node->height;
      int i;
      for (i = 1; i < height; ++i) {
        // if (!node->nexts[i].node) {
        //   break;
        // }

        if (skipped + node->nexts[i].skip_size >= from) {
          // Too far. Look at the next node's skip list.
          break;
        }
      }

      // Record how many chars we skipped.
      skipped += node->nexts[i - 1].skip_size;
      node = node->nexts[i - 1].node;

      if (!node) {
        // Went too far, can't read anything.
        *bufsize = 0;
        return 0;
      }
    }

    size_t total_bytes;
    size_t total_chars;
    char *str;
    size_t bytes;
    size_t chars;

    // Copy from the first node. At this point, we're copying to the beginning
    // of the buffer from some offset within this node.
    str = reinterpret_cast<char *>(node->str);
    char *start = detail::skip_utf8(str, from - skipped, node->num_bytes);
    chars = to - from;
    bytes = std::min(*bufsize, size_t(node->num_bytes - (start - str)));
    bytes = detail::copy_utf8(buf, start, &chars, bytes);
    from += chars;
    total_bytes = bytes;
    total_chars = chars;
    node = node->nexts[0].node;

    // Now copy from the rest of the nodes. Here we always start at the
    // beginning of the node and copy into the buffer at an offset.
    while (node && from < to && total_bytes < *bufsize) {
      str = reinterpret_cast<char *>(node->str);
      chars = to - from;
      bytes = std::min(*bufsize - total_bytes, size_t(node->num_bytes));
      bytes = detail::copy_utf8(buf + total_bytes, str, &chars, bytes);
      from += chars;
      total_bytes += bytes;
      total_chars += chars;
      node = node->nexts[0].node;
    }

    *bufsize = total_bytes;
    return total_chars;
  }

  /// Reads a substring from the rope into a buffer, including a NUL ('\0')
  /// character at the end.
  /// \see read.
  size_t read_cstr(char *buf, size_t *bufsize, size_t from, size_t to) const {
    --*bufsize;
    auto chars = read(buf, bufsize, from, to);
    buf[*bufsize] = 0;
    ++*bufsize;
    return chars + 1;
  }

  /// Get one Unicode character from the document.
  /// Note: Works in log(N) time! Try to read more in chunks for better perf.
  /// \param index UTF-8 character offset.
  /// \return The UTF-32 character at the specified offset.
  char32_t operator[](size_t index) const {
    // TODO: Read any unicode sequence (6 chars max), convert to UTF-32.
    char ch = 0;
    size_t bufsize = 1;
    read(&ch, &bufsize, index, index + 1);
    return ch;
  }

  template<typename... Args>
  void set_attribute(size_t start, size_t end, Args &&...args) {
    _attrs.insert(start, end, std::forward<Args>(args)...);
  }

  const collections::IntervalTree<A> &attributes() const {
    return _attrs;
  }

  // The document always has at least one line.
  size_t lines() const {
    return _newlines.size() + 1;
  }

  // Returns a 1-based line index.
  size_t line_at(size_t pos) const {
    if (_newlines.empty()) {
      return 1;
    }
    auto ln = _newlines.upper_bound(pos);
    if (ln == _newlines.end()) {
      return _newlines.size() + 1;
    }
    return _newlines.index_for(ln) + 1;
  }

  // Line numbers start at 1. The end of the span is the index of the newline
  // character, or for the last line it is the character length of the document.
  std::pair<size_t, size_t> span_for_line(size_t line) const {
    size_t start;
    size_t end;

    // Make it zero-based.
    --line;

    auto newlines = _newlines.size();
    if (line > newlines) {
      // Line doesn't exist - index too high.
      start = end = npos;
    } else if (newlines == 0) {
      // Whole document is one line.
      start = 0;
      end = char_length();
    } else if (line == 0) {
      // First line starts at char 0.
      start = 0;
      end = _newlines[0];
    } else if (line == newlines) {
      // Last line ends at the last character of the document.
      start = _newlines[newlines - 1] + 1;
      end = char_length();
    } else {
      // All other lines span from [previous newline + 1, next newline].
      auto it = _newlines.get(line - 1);
      start = *it + 1;
      end = *++it;
    }

    assert(end >= start);
    return std::make_pair(start, end);
  }

private:
  rope *_text;
  collections::SlidingOrderedSet<> _newlines;
  collections::IntervalTree<A> _attrs;
};

} // namespace ash::doc

#endif // ASH_DOCUMENT_H_
