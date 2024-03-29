#ifndef LAVA_UTIL_UTF_CP_H_
#define LAVA_UTIL_UTF_CP_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

constexpr const char32_t invalid_char = static_cast<char32_t>(-1);

/// \return the length in bytes of the UTF-8 encoded codepoint starting with
/// \c byte, from 1 to 6, or 0 if \c byte is invalid as a codepoint starting
/// byte.
inline size_t utf8_codepoint_size(uint8_t byte) {
  if (byte <= 0x7f) { return 1; } // 0x74 = 0111 1111
  else if (byte <= 0xbf) { return 0; } // 1011 1111. Invalid for a starting byte.
  else if (byte <= 0xdf) { return 2; } // 1101 1111
  else if (byte <= 0xef) { return 3; } // 1110 1111
  else if (byte <= 0xf7) { return 4; } // 1111 0111
  else if (byte <= 0xfb) { return 5; } // 1111 1011
  else if (byte <= 0xfd) { return 6; } // 1111 1101
  else { return 1; }
}

/// \return The UTF-32 character at \c *buf, or \c invalid_char if \c *buf is
/// invalid.
inline char32_t utf8_to_utf32(const char *buf) {
  char32_t ch = 0;
  // 6: x011 1111 2222 2233 3333 4444 4455 5555
  // 5: xxxx xx00 1111 1122 2222 3333 3344 4444
  // 4: xxxx xxxx xxx0 0011 1111 2222 2233 3333
  // 3: xxxx xxxx xxxx xxxx 0000 1111 1122 2222
  // 2: xxxx xxxx xxxx xxxx xxxx x000 0011 1111
  // 1: xxxx xxxx xxxx xxxx xxxx xxxx x000 0000
  char32_t mask = 0x7f;
  size_t bufsize = utf8_codepoint_size(*buf);
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

/// Convert the UTF-32 character \c to UTF-8, writing the result into \c buf.
/// \requires sizeof(*buf) >= 6
/// \returns The size in bytes of the UTF-8 sequence in \c buf.
/// TODO: Detect invalid characters.
inline size_t utf32_to_utf8(char32_t c, char *buf) {
  if (c < 0x80) {
    buf[0] = char(c);
    return 1;
  }

  if (c < 0x800) {
    buf[0] = char((c >> 6)   | 0xc0);
    buf[1] = char((c & 0x3f) | 0x80);
    return 2;
  }

  if (c < 0x10000) {
    buf[0] = char( (c >> 12)         | 0xe0);
    buf[1] = char(((c >>  6) & 0x3f) | 0x80);
    buf[2] = char( (c & 0x3f)        | 0x80);
    return 3;
  }

  if (c < 0x200000) {
    buf[0] = char( (c >> 18)         | 0xf0);
    buf[1] = char(((c >> 12) & 0x3f) | 0x80);
    buf[2] = char(((c >>  6) & 0x3f) | 0x80);
    buf[3] = char( (c        & 0x3f) | 0x80);
    return 4;
  }

  if (c < 0x4000000) {
    buf[0] = char( (c >> 24)         | 0xf8);
    buf[1] = char(((c >> 18) & 0x3f) | 0x80);
    buf[2] = char(((c >> 12) & 0x3f) | 0x80);
    buf[3] = char(((c >> 6)  & 0x3f) | 0x80);
    buf[4] = char( (c        & 0x3f) | 0x80);
    return 5;
  }

  buf[0] = char(((c >> 30) &    1) | 0xfc);
  buf[1] = char(((c >> 24) & 0x3f) | 0x80);
  buf[2] = char(((c >> 18) & 0x3f) | 0x80);
  buf[3] = char(((c >> 12) & 0x3f) | 0x80);
  buf[4] = char(((c >> 6)  & 0x3f) | 0x80);
  buf[5] = char( (c        & 0x3f) | 0x80);
  return 6;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LAVA_UTIL_UTF_CP_H_ */
