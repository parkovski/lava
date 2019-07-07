#include "ash/ash.h"
#include "ash/terminal/terminal.h"
#include "ash/terminal/lineeditor.h"
#include "ash/collections/intervaltree.h"

#include "rope/rope.h"

#include <iostream>
#include <memory>
#include <type_traits>
#include <cassert>

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
    self->_descriptor = descriptor;
    return self;
  }

  AttributeData *create(size_t size, const void *data, symbol_t descriptor)
    noexcept
  {
    auto self = create(size, descriptor);
    memcpy(self->data(), data, size);
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

class Document {
public:
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
    assert(to >= from);
    rope_del(_text, from, to - from);
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

  void set_attribute() {
  }

private:
  rope *_text;
  ash::collections::IntervalTree<AttributeData> _tree;
};

bool readpair(const std::string &s, size_t &first, size_t &second) {
# define SKIPWS while (s[index] == ' ') { ++index; }
  size_t index = 0;
  first = 0;
  second = 0;
  SKIPWS
  if (s[index] < '0' || s[index] > '9') {
    return false;
  }
  while (s[index] >= '0' && s[index] <= '9') {
    first = first * 10 + s[index] - '0';
    ++index;
  }
  SKIPWS
  if (s[index] == ',') {
    ++index;
    SKIPWS
  }
  if (s[index] < '0' || s[index] > '9') {
    return false;
  }
  while (s[index] >= '0' && s[index] <= '9') {
    second = second * 10 + s[index] - '0';
    ++index;
  }
  return true;
# undef SKIPWS
}

int main(int argc, char *argv[]) {
  ash::term::initialize();
  ASH_SCOPEEXIT { ash::term::restoreState(); };

  ash::LineEditor ed;
  std::string line;
  Document doc;

  doc.append(
    "abcdefghijklmnopqrstuvwxy"
    "ABCDEFGHIJKLMNOPQRSTUVWXY"
    "abcdefghijklmnopqrstuvwxy"
    "ABCDEFGHIJKLMNOPQRSTUVWXY"
    "hello world hello world 1"
    "abcdefghijklmnopqrstuvwxy"
    "ABCDEFGHIJKLMNOPQRSTUVWXY"
    "abcdefghijklmnopqrstuvwxy"
    "ABCDEFGHIJKLMNOPQRSTUVWXY"
    "hello world hello world 2"
    "abcdefghijklmnopqrstuvwxy"
    "ABCDEFGHIJKLMNOPQRSTUVWXY"
    "abcdefghijklmnopqrstuvwxy"
    "ABCDEFGHIJKLMNOPQRSTUVWXY"
    "hello world hello world 3"
    "abcdefghijklmnopqrstuvwxy"
    "ABCDEFGHIJKLMNOPQRSTUVWXY"
    "abcdefghijklmnopqrstuvwxy"
    "ABCDEFGHIJKLMNOPQRSTUVWXY"
    "hello world hello world 4"
    "abcdefghijklmnopqrstuvwxy"
    "ABCDEFGHIJKLMNOPQRSTUVWXY"
    "abcdefghijklmnopqrstuvwxy"
    "ABCDEFGHIJKLMNOPQRSTUVWXY"
    "hello world hello world 5"
  );

  std::cout << "Chars = " << doc.char_length()
            << "; bytes = " << doc.byte_length() << ".\n";
  while (ed.readLine(line)) {
    size_t first, second;
    if (!readpair(line, first, second)) {
      std::cout << "lol no\n";
      continue;
    }
    std::cout << "Reading (" << first << ", " << second << ").\n";
    size_t bufsize = second - first + 1;
    line.reserve(bufsize);
    auto read = doc.read_cstr(line.data(), &bufsize, first, second) - 1;
    std::cout << "Read " << read << " chars.\n" << line.data() << "\n\n";
  }

  return 0;
}
