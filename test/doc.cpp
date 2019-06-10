#include "ash/ash.h"
#include "ash/terminal/terminal.h"
#include "ash/terminal/lineeditor.h"

#define USE_INTERVAL_TREE_NAMESPACE
#include "intervaltree/IntervalTree.h"

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

class Document {
public:
  explicit Document() noexcept
  {
    _text = rope_new();
  }

  ~Document() {
    rope_free(_text);
  }

  void insert(size_t pos, const char *text) {
    rope_insert(_text, pos, reinterpret_cast<const uint8_t *>(text));
  }

  size_t length() const {
    return rope_byte_count(_text);
  }

  void append(const char *text) {
    insert(this->length(), text);
  }

  void erase(size_t from, size_t to) {
    assert(to >= from);
    rope_del(_text, from, to - from);
  }

  void replace(size_t from, size_t to, const char *text) {
    erase(from, to);
    insert(from, text);
  }

  size_t read(char *buf, size_t from, size_t to) const {
    auto len = length();
    if (to > len) {
      to = len;
    }
    if (from >= to) {
      return 0;
    }

    rope_node *node = &_text->head;
    // Bytes!
    size_t skipped = 0;

    // Find the starting node.
    while (skipped + node->num_bytes < from) {
      // Use the skip list.
      auto height = node->height;
      int i;
      for (i = 1; i < height; ++i) {
        if (!node->nexts[i].node) {
          break;
        }

        // UTF-8 characters!
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

    // Copy into the buffer.
    auto rope_offset = from - skipped;
    size_t bytes = node->num_bytes - rope_offset;
    if (to - from < bytes) {
      // Less to read than the full string in the node.
      bytes = to - from;
    }
    memcpy(buf, node->str + rope_offset, bytes);
    from += bytes;
    node = node->nexts[0].node;

    size_t buf_offset = bytes;
    while (node && from < to) {
      bytes = node->num_bytes;
      if (to - from < bytes) {
        bytes = to - from;
      }
      memcpy(buf + buf_offset, node->str, bytes);
      from += bytes;
      buf_offset += bytes;
      node = node->nexts[0].node;
    }

    return buf_offset;
  }

  // Note: Works in log(N) time! Try to read more in chunks for better perf.
  char operator[](size_t index) const {
    char ch = 0;
    read(&ch, index, index + 1);
    return ch;
  }

private:
  // NOTE! This thing uses UTF-8 character indexes sometimes.
  rope *_text;
  interval_tree::IntervalTree<size_t, AttributeData *> _tree;
};

bool readpair(const std::string &s, size_t &first, size_t &second) {
# define SKIPWS while (s[index] == ' ') { ++index; }
  size_t index = 0;
  first = 0;
  second = 0;
  SKIPWS
  while (s[index] >= '0' && s[index] <= '9') {
    first = first * 10 + s[index] - '0';
    ++index;
  }
  SKIPWS
  if (s[index] == ',') {
    ++index;
    SKIPWS
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

  std::cout << "Length = " << doc.length() << ".\n";
  while (ed.readLine(line)) {
    size_t first, second;
    readpair(line, first, second);
    std::cout << "Reading (" << first << ", " << second << ").\n";
    line.reserve(second - first + 1);
    auto read = doc.read(line.data(), first, second);
    line.data()[second - first] = 0;
    std::cout << "Read " << read << " chars.\n" << line.data() << "\n\n";
  }

  return 0;
}
