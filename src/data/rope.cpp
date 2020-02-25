#include "ash/ash.h"
#define ASH_C_ROPE_FWD
#include "ash/data/rope.h"

#include <rope/rope.h>

using namespace ash;
using namespace ash::data::rope;

namespace {

  /// Skip to the given UTF-8 offset in the string.
  /// \param str The string to search.
  /// \param chars The number of characters to skip.
  /// \param bytelen The length of the string in bytes.
  /// \return A pointer to the new offset in the string.
  static inline char *skip_utf8(char *str, size_t chars, size_t bytelen) {
    size_t chars_seen = 0;
    const char *const end = str + bytelen;
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
  static inline size_t copy_utf8(char *dst, const char *src, size_t *chars,
                                 size_t max_size)
  {
    size_t chars_seen = 0;
    const char *const start = src;
    const char *const end = src + max_size;
    const char *const safe_end = end - 6;

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
  : _c_rope(rope_new())
{}

Rope::Rope(std::string_view text)
  : _c_rope(rope_new_with_utf8_n(reinterpret_cast<const uint8_t *>(text.data()),
                               text.length()))
{
  if (!_c_rope) {
    throw std::bad_alloc{};
  }
}

Rope::Rope(const char_type *text)
  : _c_rope(rope_new_with_utf8(reinterpret_cast<const uint8_t *>(text)))
{
  if (!_c_rope) {
    throw std::bad_alloc{};
  }
}

Rope::Rope(const Rope &other)
  : _c_rope(rope_copy(other._c_rope))
{
  if (!_c_rope) {
    throw std::bad_alloc{};
  }
}

Rope &Rope::operator=(const Rope &other) {
  rope_free(_c_rope);
  if (!(_c_rope = rope_copy(other._c_rope))) {
    throw std::bad_alloc{};
  }
  return *this;
}

Rope::Rope(Rope &&other) noexcept
  : _c_rope(other._c_rope)
{
  other._c_rope = nullptr;
}

Rope &Rope::operator=(Rope &&other) noexcept {
  rope_free(_c_rope);
  _c_rope = other._c_rope;
  other._c_rope = nullptr;
  return *this;
}

Rope::~Rope() {
  if (_c_rope) {
    rope_free(_c_rope);
  }
}

bool Rope::insert(size_t index, std::string_view text) {
  if (rope_insert_n(_c_rope, index,
                    reinterpret_cast<const uint8_t *>(text.data()),
                    text.length()) != ROPE_OK) {
    return false;
  }

  return true;
}

bool Rope::insert(size_t index, const char_type *text) {
  if (rope_insert(_c_rope, index, reinterpret_cast<const uint8_t *>(text)) !=
      ROPE_OK) {
    return false;
  }

  return true;
}

size_t Rope::length() const {
  return rope_char_count(_c_rope);
}

size_t Rope::size() const {
  return rope_byte_count(_c_rope);
}

size_t Rope::u16_length() const {
  return rope_wchar_count(_c_rope);
}

bool Rope::append(std::string_view text) {
  return insert(this->length(), text);
}

bool Rope::append(const char_type *text) {
  return insert(this->length(), text);
}

void Rope::erase(size_t index, size_t count) {
  rope_del(_c_rope, index, count);
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
  rope_del(_c_rope, 0, length());
}

size_t Rope::substr(char_type *buf, size_t *bufsize, size_t index, size_t count)
                    const {
  return rope_write_substr(_c_rope, reinterpret_cast<uint8_t *>(buf), bufsize,
                           index, count);
}

size_t Rope::c_substr(char_type *buf, size_t *bufsize, size_t index,
                      size_t count) const {
  --*bufsize;
  auto chars = substr(buf, bufsize, index, count);
  buf[*bufsize] = 0;
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
    bufsize = sizeof(buf);
  }

  return s;
}

char32_t Rope::operator[](size_t index) const {
  char_type ch[6];
  size_t bufsize = 6;
  substr(ch, &bufsize, index, 1);
  return utf8_to_utf32(ch);
}
