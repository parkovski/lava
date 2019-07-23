#include "ash/ash.h"
#include "ash/terminal/terminal.h"
#include "ash/terminal/lineeditor.h"
#include "ash/collections/redblacktree.h"
#include "ash/collections/intervaltree.h"

#include "rope/rope.h"

#include <string_view>
#include <string>
#include <iostream>
#include <memory>
#include <type_traits>
#include <cassert>
#include <deque>

// Missing from C++17 and below.
#ifdef _MSVC_LANG
#define _lang_ _MSVC_LANG
#else
#define _lang_ __cplusplus
#endif

#if _lang_ <= 201703L
namespace std {
  template<class T>
  struct remove_cvref {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
  };

  template<class T>
  using remove_cvref_t = typename remove_cvref<T>::type;
}
#endif

template<typename T>
struct symbol_traits;

typedef unsigned symbol_t;

template<typename T>
inline constexpr symbol_t descriptor_v
  = symbol_traits<std::remove_cvref_t<T>>::descriptor;

template<> struct symbol_traits<size_t> {
  static constexpr symbol_t descriptor = symbol_t(1);
};

template<> struct symbol_traits<char *> {
  static constexpr symbol_t descriptor = symbol_t(2);
};

class AttributeData {
private:
  symbol_t _descriptor;
  //char _data[];

public:
  AttributeData() = delete;

  AttributeData(const AttributeData &) = delete;
  AttributeData &operator=(const AttributeData &) = delete;

  AttributeData(AttributeData &&) = delete;
  AttributeData &operator=(AttributeData &&) = delete;

  AttributeData *create(size_t size, symbol_t descriptor) noexcept {
    auto self = reinterpret_cast<AttributeData *>(
      ::operator new(sizeof(AttributeData) + size, std::nothrow)
    );
    if (self) {
      self->_descriptor = descriptor;
    }
    return self;
  }

  AttributeData *create(size_t size, const void *data, symbol_t descriptor)
    noexcept
  {
    auto self = create(size, descriptor);
    if (self) {
      memcpy(self->data(), data, size);
    }
    return self;
  }

  template<
    typename T,
    typename = std::enable_if_t<std::is_nothrow_move_constructible_v<T>>
  >
  AttributeData *create(T &&data, symbol_t descriptor) noexcept {
    auto self = create(sizeof(T), descriptor);
    *reinterpret_cast<T *>(self->data()) = std::move(data);
    return self;
  }

  template<
    typename T,
    auto D = descriptor_v<T>,
    typename = std::enable_if_t<std::is_nothrow_move_constructible_v<T>>
  >
  AttributeData *create(T &&data) noexcept {
    return create(std::move(data), D);
  }

  ~AttributeData()
  { ::operator delete(this); }

  symbol_t descriptor() const
  { return _descriptor; }

  template<typename T = void>
  const T *data() const noexcept
  { return reinterpret_cast<const T *>(this + 1); }

  template<typename T = void>
  T *data() noexcept
  { return reinterpret_cast<T *>(this + 1); }
};


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

  while (src < safe_end) {
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

  while (src < end) {
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

static size_t rope_read(rope *rope, char *buf, size_t *bufsize, size_t from,
                        size_t to) {
  auto len = rope_char_count(rope);
  if (to > len) {
    to = len;
  }

  rope_node *node = &rope->head;
  size_t skipped = 0;

  // Find the starting node.
  while (skipped + node->nexts[0].skip_size < from) {
    // Use the skip list.
    auto height = node->height;
    int i;
    for (i = 1; i < height; ++i) {
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
  char *start = skip_utf8(str, from - skipped, node->num_bytes);
  chars = to - from;
  bytes = std::min(*bufsize, size_t(node->num_bytes - (start - str)));
  bytes = copy_utf8(buf, start, &chars, bytes);
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
    bytes = copy_utf8(buf + total_bytes, str, &chars, bytes);
    from += chars;
    total_bytes += bytes;
    total_chars += chars;
    node = node->nexts[0].node;
  }

  *bufsize = total_bytes;
  return total_chars;
}

static char32_t utf_codepoint(char *buf, size_t bufsize) {
  const char32_t invalid_char = static_cast<char32_t>(-1);
  char32_t ch = 0;
  // 6: x011 1111 2222 2233 3333 4444 4455 5555
  // 5: xxxx xx00 1111 1122 2222 3333 3344 4444
  // 4: xxxx xxxx xxx0 0011 1111 2222 2233 3333
  // 3: xxxx xxxx xxxx xxxx 0000 1111 1122 2222
  // 2: xxxx xxxx xxxx xxxx xxxx x000 0011 1111
  // 1: xxxx xxxx xxxx xxxx xxxx xxxx x000 0000
  char32_t mask = 0x7f;
  switch (bufsize) {
    case 6:
      ch = buf[0] << 24;
    case 5:
      ch |= (buf[bufsize - 5] & 0x3f) << 18;
      mask <<= 5;
      mask |= 0x1f;
    case 4:
      ch |= (buf[bufsize - 4] & 0x3f) << 12;
      mask <<= 5;
      mask |= 0x1f;
    case 3:
      ch |= (buf[bufsize - 3] & 0x3f) << 6;
      mask <<= 5;
      mask |= 0x1f;
    case 2:
      ch |= buf[bufsize - 2] & 0x3f;
      ch <<= 6;
      mask <<= 5;
      mask |= 0x1f;
    case 1:
      ch |= buf[bufsize - 1] & 0x7f;
      mask <<= 4;
      mask |= 0xf;
      return ch & mask;
    default:
      return invalid_char;
  }
}

template<typename A>
class Document {
public:
  using attribute_tree = ash::collections::IntervalTree<A>;
  using line_nr_tree = ash::collections::RedBlackTree<size_t>;

  /// Construct an empty document.
  explicit Document() noexcept {
    _text = rope_new();
  }

  /// Construct a document pre-populated with text.
  /// \param text The text to fill the document with.
  explicit Document(const char *text) noexcept {
    _text = rope_new_with_utf8(reinterpret_cast<const uint8_t *>(text));
  }

  ~Document() {
    rope_free(_text);
  }

private:
  void insert_lines(size_t first, std::string_view text) {
  }

public:
  /// Insert text into the document.
  /// \param pos UTF-8 character position.
  /// \param text UTF-8 string to insert.
  void insert(size_t pos, const char *text) {
    rope_insert(_text, pos, reinterpret_cast<const uint8_t *>(text));
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
    assert(to > from);
    rope_del(_text, from, to - from);
  }

  /// Replace a range with different text. Indexes in characters.
  /// \param from The beginning of the range to delete, inclusive.
  /// \param to The end of the range to delete, exclusive.
  /// \param text The new text to insert at \c from.
  void replace(size_t from, size_t to, const char *text) {
    assert(to > from);
    rope_del(_text, from, to - from);
    rope_insert(_text, from, reinterpret_cast<const uint8_t *>(text));
  }

  /// Read a range of text from the rope into a buffer. Indexes in characters.
  /// Does _not_ append a null to the buffer.
  /// \param buf The buffer to receive the text.
  /// \param bufsize The size of the buffer in bytes. On return, this will be
  ///                set to the number of bytes written to the buffer.
  /// \param from The beginning of the range to read from, inclusive.
  /// \param to The end of the range to read from, exclusive. Must be greater
  ///           than \c from.
  /// \return The number of characters written to the buffer.
  size_t read(char *buf, size_t *bufsize, size_t from, size_t to) const {
    return rope_read(_text, buf, bufsize, from, to);
  }

  /// Reads a substring from the document.
  /// \param buf A string to append the document substring to.
  /// \param from The starting point in the document.
  /// \param to The ending point (exclusive) in the document.
  void read(std::string &buf, size_t from, size_t to) const {
    char chbuf[128];
    while (from < to) {
      size_t bytes = sizeof(buf);
      from += read(chbuf, &bytes, from, to);
      buf.append(chbuf, chbuf + bytes);
    }
  }

  /// Reads a substring from the rope into a buffer, including a NUL ('\0')
  /// character at the end.
  /// \see read.
  size_t read_cstr(char *buf, size_t *bufsize, size_t from, size_t to) const {
    --*bufsize;
    auto bytes = read(buf, bufsize, from, to);
    buf[*bufsize] = 0;
    ++*bufsize;
    return bytes + 1;
  }

  /// Get one Unicode code point from the document.
  /// Note: Works in log(N) time! Try to read more in chunks for better perf.
  /// \param index UTF-8 character offset.
  /// \return The UTF-32 character at the specified offset.
  char32_t operator[](size_t index) const {
    char bytes[6];
    size_t bufsize = 6;
    read(bytes, &bufsize, index, index + 1);
    return utf_codepoint(bytes, bufsize);
  }

  unsigned line(size_t index) const {
    return 0;
  }

  std::pair<size_t, size_t> span_for_line(unsigned line) const {
    return std::make_pair<size_t, size_t>(0, 10);
  }

  template<typename... Args>
  A &set_attribute(size_t start, size_t end, Args &&...args) {
    return _attrs.insert(start, end, std::forward<Args>(args)...);
  }

  void remove_attribute(typename attribute_tree::const_iterator where) {
    _attrs.erase(where);
  }

  typename attribute_tree::inner_search_iterator
  find_inner_attributes(size_t start, size_t end) {
    return _attrs.find_inner(start, end);
  }

  typename attribute_tree::const_inner_search_iterator
  find_inner_attributes(size_t start, size_t end) const {
    return _attrs.find_inner(start, end);
  }

  typename attribute_tree::overlap_search_iterator
  find_attributes(size_t start, size_t end = 0) {
    if (end == 0) {
      end = start + 1;
    }
    return _attrs.find_overlap(start, end);
  }

  typename attribute_tree::const_overlap_search_iterator
  find_attributes(size_t start, size_t end = 0) const {
    if (end == 0) {
      end = start + 1;
    }
    return _attrs.find_overlap(start, end);
  }

  typename attribute_tree::equal_search_iterator
  find_exact_attributes(size_t start, size_t end) {
    return _attrs.find_equal(start, end);
  }

  typename attribute_tree::const_equal_search_iterator
  find_exact_attributes(size_t start, size_t end) const {
    return _attrs.find_equal(start, end);
  }

  typename attribute_tree::iterator
  attribute_begin() {
    return _attrs.begin();
  }

  typename attribute_tree::const_iterator
  attribute_begin() const {
    return _attrs.begin();
  }

  typename attribute_tree::iterator
  attribute_end() {
    return _attrs.end();
  }

  typename attribute_tree::const_iterator
  attribute_end() const {
    return _attrs.end();
  }

private:
  rope *_text;
  attribute_tree _attrs;
  line_nr_tree _line_nrs;
};

struct line_and_color {
  unsigned line;
  unsigned color;
};

#include <cstdlib>
int main(int argc, char *argv[]) {
  ash::term::initialize();
  ASH_SCOPEEXIT { ash::term::restoreState(); };

  ash::LineEditor ed;
  std::string line;
  Document<line_and_color> doc;

  while (ed.readLine(line)) {
    if (line.length() == 0) {
      continue;
    }
    if (line[0] == '#' && line.length() > 1) {
      auto nr = atoi(&line[1]);
      if (nr > 0 || line[1] == '0') {
        auto [start, end] = doc.span_for_line(nr);
        line.clear();
        doc.read(line, start, end);
        std::cout << line << "\n";
      }
    }
    line.push_back('\n');
    doc.append(line.c_str());
  }

  return 0;
}
