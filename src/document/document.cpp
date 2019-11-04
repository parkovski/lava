#include "rope/rope.h"
#define ASH_DOCUMENT_IMPL
#include "ash/document.h"

#include <new>

using namespace ash;
using namespace ash::doc;

namespace {

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


} // anonymous namespace

Rope::Rope()
  : _text(rope_new())
{}

Rope::Rope(std::string_view text)
  : _text(rope_new_with_utf8_n(reinterpret_cast<const uint8_t *>(text.data()),
                               text.length()))
{
  if (!_text) {
    throw std::bad_alloc{};
  }
}

Rope::Rope(const char_type *text)
  : _text(rope_new_with_utf8(reinterpret_cast<const uint8_t *>(text)))
{
  if (!_text) {
    throw std::bad_alloc{};
  }
}

Rope::Rope(const Rope &other)
  : _text(rope_copy(other._text))
{
  if (!_text) {
    throw std::bad_alloc{};
  }
}

Rope &Rope::operator=(const Rope &other) {
  rope_free(_text);
  if (!(_text = rope_copy(other._text))) {
    throw std::bad_alloc{};
  }
  return *this;
}

Rope::Rope(Rope &&other) noexcept
  : _text(other._text)
{
  other._text = nullptr;
}

Rope &Rope::operator=(Rope &&other) noexcept {
  rope_free(_text);
  _text = other._text;
  other._text = nullptr;
  return *this;
}

Rope::~Rope() {
  if (_text) {
    rope_free(_text);
  }
}

bool Rope::insert(size_t index, std::string_view text) {
  if (rope_insert_n(_text, index,
                    reinterpret_cast<const uint8_t *>(text.data()),
                    text.length()) != ROPE_OK) {
    return false;
  }

  return true;
}

bool Rope::insert(size_t index, const char_type *text) {
  if (rope_insert(_text, index, reinterpret_cast<const uint8_t *>(text)) !=
      ROPE_OK) {
    return false;
  }

  return true;
}

size_t Rope::length() const {
  return rope_char_count(_text);
}

size_t Rope::size() const {
  return rope_byte_count(_text);
}

size_t Rope::u16_length() const {
  return rope_wchar_count(_text);
}

bool Rope::append(const char_type *text) {
  return insert(this->length(), text);
}

void Rope::erase(size_t index, size_t count) {
  rope_del(_text, index, count);
}

bool Rope::replace(size_t index, size_t count, std::string_view text) {
  erase(index, count);
  return insert(index, text);
}

bool Rope::replace(size_t index, size_t count, const char_type *text) {
  erase(index, count);
  return insert(index, text);
}

void Rope::clear() {
  rope_del(_text, 0, length());
}

size_t Rope::substr(char_type *buf, size_t *bufsize, size_t index, size_t count)
                    const {
  auto len = length();
  if (index > len) {
    *bufsize = 0;
    return 0;
  }
  if (index + count > len) {
    count = len - index;
  }

  rope_node *node = &_text->head;
  size_t skipped = 0;

  // Find the starting node.
  while (skipped + node->nexts[0].skip_size < index) {
    // Use the skip list.
    auto height = node->height;
    int i;
    for (i = 1; i < height; ++i) {
      // if (!node->nexts[i].node) {
      //   break;
      // }

      if (skipped + node->nexts[i].skip_size >= index) {
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
  char_type *str;
  size_t bytes;
  size_t chars;

  // Copy from the first node. At this point, we're copying to the beginning
  // of the buffer from some offset within this node.
  str = reinterpret_cast<char_type *>(node->str);
  char_type *start = skip_utf8(str, index - skipped, node->num_bytes);
  chars = count;
  bytes = std::min(*bufsize, size_t(node->num_bytes - (start - str)));
  bytes = copy_utf8(buf, start, &chars, bytes);
  index += chars;
  count -= chars;
  total_bytes = bytes;
  total_chars = chars;
  node = node->nexts[0].node;

  // Now copy from the rest of the nodes. Here we always start at the
  // beginning of the node and copy into the buffer at an offset.
  while (node && count > 0 && total_bytes < *bufsize) {
    str = reinterpret_cast<char_type *>(node->str);
    chars = count;
    bytes = std::min(*bufsize - total_bytes, size_t(node->num_bytes));
    bytes = copy_utf8(buf + total_bytes, str, &chars, bytes);
    index += chars;
    count -= chars;
    total_bytes += bytes;
    total_chars += chars;
    node = node->nexts[0].node;
  }

  *bufsize = total_bytes;
  return total_chars;
}

size_t Rope::c_substr(char_type *buf, size_t *bufsize, size_t index,
                      size_t count) const {
  --*bufsize;
  auto chars = substr(buf, bufsize, index, count);
  buf[*bufsize] = 0;
  ++*bufsize;
  return chars;
}

std::string Rope::substr(size_t index, size_t count) const {
  // HACK
  char_type buf[512];
  std::string s;
  size_t bufsize = sizeof(buf);
  while (auto copied = substr(buf, &bufsize, index, count)) {
    index += copied;
    count -= copied;
    s.append(buf, bufsize);
  }

  return s;
}

char32_t Rope::operator[](size_t index) const {
  char_type ch[6];
  size_t bufsize = 6;
  substr(ch, &bufsize, index, 1);
  return utf8_to_utf32(ch);
}

Document::Document()
  : _rope()
{}

Document::Document(const u8char *text)
  : _rope(text)
{
  markNewlines(0, text);
}

Document::~Document()
{}

void Document::markNewlines(size_t index, const char_type *text) {
  size_t len = 0;
  size_t i = 0;
  char_type ch = text[0];
  while (ch) {
    if (ch == '\n') {
      _newlines.insert(index + i);
    }
    ++len;
    i += utf8_codepoint_size(ch);
     ch = text[i];
  }
}

bool Document::insert(size_t index, const char_type *text) {
  auto old_len = length();
  auto old_size = size();
  if (!_rope.insert(index, text)) {
    return false;
  }
  auto delta_len = length() - old_len;
  auto delta_size = size() - old_size;
  _newlines.shift(index, delta_len);
  markNewlines(index, text);
  emit(Insert(index, text, delta_len, delta_size));
  return true;
}

size_t Document::length() const {
  return _rope.length();
}

size_t Document::size() const {
  return _rope.size();
}

bool Document::append(const char_type *text) {
  auto old_length = length();
  auto old_size = size();
  auto r = _rope.append(text);
  emit(Insert(old_length, text, length() - old_length, size() - old_size));
  return r;
}

void Document::erase(size_t index, size_t count) {
  auto old_size = size();
  _rope.erase(index, count);
  _newlines.shift(index, -static_cast<ptrdiff_t>(count));
  emit(Erase(index, count, old_size - size()));
}

bool Document::replace(size_t index, size_t count, const char_type *text) {
  auto old_length = length();
  //auto old_size = static_cast<ptrdiff_t>(size());
  if (index >= old_length) {
    return insert(index, text);
  }
  if (index + count > old_length) {
    count = old_length - index;
  }
  if (!_rope.replace(index, count, text)) {
    return false;
  }

  // old - count + n = new;
  // n = new + count - old;
  auto inserted = length() + count - old_length;
  _newlines.shift(index, -static_cast<ptrdiff_t>(count));
  _newlines.shift(index, inserted);
  emit(Replace(index, count, text, inserted));
  return true;
}

void Document::clear() {
  auto old_length = length();
  auto old_size = size();
  _rope.clear();
  _newlines.clear();
  emit(Erase(0, old_length, old_size));
}

size_t Document::substr(char_type *buf, size_t *bufsize, size_t index,
                        size_t count) const {
  return _rope.substr(buf, bufsize, index, count);
}

size_t Document::c_substr(char_type *buf, size_t *bufsize, size_t index,
                         size_t count) const {
  return _rope.c_substr(buf, bufsize, index, count);
}

std::string Document::substr(size_t index, size_t count) const {
  return _rope.substr(index, count);
}

char32_t Document::operator[](size_t index) const {
  return _rope[index];
}

// The document always has at least one line.
Document::grid_size_type Document::lines() const {
  return _newlines.size() + 1;
}

// Returns a 1-based line index.
Document::grid_size_type Document::lineAt(size_t index) const {
  if (_newlines.empty()) {
    return 1;
  }
  auto ln = _newlines.upper_bound(index);
  if (ln == _newlines.end()) {
    return _newlines.size() + 1;
  }
  return _newlines.index_for(ln) + 1;
}

// Make line and column a valid pair if either is out of range.
void Document::constrain(grid_size_type &line, grid_size_type &column) const {
  if (line == 0) {
    line = 1;
  }
  if (line > _newlines.size()) {
    line = _newlines.size() + 1;
  }
  if (column == 0) {
    column = 1;
  }
  if (_newlines.size() == 0) {
    line = 1;
    if (column > length()) {
      column = length() + 1;
    }
    return;
  }

  auto next_nl = _newlines.get(line - 1);
  if (next_nl == _newlines.end()) {
    auto first_col = *_newlines.rbegin() + 1;
    auto max_col = length() - first_col + 1;
    if (column > max_col) {
      column = max_col;
    }
    return;
  }

  auto prev_nl = next_nl - 1;
  auto max_col = *next_nl - *prev_nl + 1;
  if (column > max_col) {
    column = max_col;
  }
}

// Returns the (line, column) pair for a character position.
// If pos is out of range, the last character position is returned.
std::pair<Document::grid_size_type, Document::grid_size_type>
Document::indexToPoint(size_t index) const {
  if (index > length()) {
    index = length();
  }
  if (_newlines.empty()) {
    return {1, index + 1};
  }

  // Newline after the current line.
  auto nl = _newlines.upper_bound(index);
  grid_size_type line, column;
  if (nl == _newlines.end()) {
    // Last line.
    line = _newlines.size() + 1;
    column = index - *_newlines.rbegin() + 1;
  } else {
    line = _newlines.index_for(nl) + 1;
    if (line == 1) {
      // First line.
      column = index + 1;
    } else {
      column = index - *(nl - 1) + 1;
    }
  }

  return {line, column};
}

// Returns a character position for a (line, column) pair.
// Out of range values are wrapped to [1, max].
size_t Document::pointToIndex(grid_size_type line, grid_size_type column) const {
  // Make indices 0-based.
  if (line > 0) {
    --line;
  }
  if (column > 0) {
    --column;
  }

  if (_newlines.empty()) {
    // Only one line in the document.
    if (column > length()) {
      return length();
    }
    return column;
  }

  // This is the newline at the _end_ of this line.
  auto nl = _newlines.get(line);
  if (nl == _newlines.end()) {
    // Last line.
    auto index = *_newlines.rbegin() + column;
    if (index > length()) {
      return length();
    }
    return index;
  } else {
    if (_newlines.index_for(nl) == 0) {
      // First line.
      if (column > *nl) {
        return *nl;
      }
      return column;
    }

    // Any middle line: the position is the column offset from the last
    // newline.
    auto index = *(nl - 1) + column;
    if (index > *nl) {
      return *nl;
    }
    return index;
  }
}

// Line numbers start at 1. The end of the span is the index of the newline
// character, or for the last line it is the character length of the document.
std::pair<size_t, size_t> Document::spanForLine(grid_size_type line) const {
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
    end = length();
  } else if (line == 0) {
    // First line starts at char 0.
    start = 0;
    end = _newlines[0];
  } else if (line == newlines) {
    // Last line ends at the last character of the document.
    start = _newlines[newlines - 1] + 1;
    end = length();
  } else {
    // All other lines span from [previous newline + 1, next newline].
    auto it = _newlines.get(line - 1);
    start = *it + 1;
    end = *++it;
  }

  assert(end >= start);
  return std::make_pair(start, end);
}
