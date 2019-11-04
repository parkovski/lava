#ifndef ASH_DATA_ROPE_H_
#define ASH_DATA_ROPE_H_

#include <cstddef>
#include <string>
#include <string_view>

// Only add this symbol to the global namespace in implementation files.
#ifdef ASH_C_ROPE_FWD
struct rope;
#endif

namespace ash::data {
namespace rope {

#ifdef ASH_C_ROPE_FWD
  using c_rope_t = ::rope;
#else
  struct c_rope_t;
#endif

// UTF-8 rope. Try to look like an std::string as much as possible.
// TODO: Iterators
class Rope final {
public:
  using char_type = char;
  using size_type = size_t;
  constexpr static size_t npos = (size_t)-1;

  /// Construct an empty document.
  explicit Rope();

  /// Construct a document pre-populated with text.
  /// \param text The text to fill the document with.
  explicit Rope(std::string_view text);
  explicit Rope(const char_type *text);

  Rope(const Rope &);
  Rope &operator=(const Rope &);
  Rope(Rope &&) noexcept;
  Rope &operator=(Rope &&) noexcept;

  ~Rope();

  /// Insert text into the document.
  /// \param index UTF-8 character position.
  /// \param text UTF-8 string to insert.
  bool insert(size_t index, std::string_view text);
  bool insert(size_t index, const char_type *text);

  // Length in UTF-8 characters.
  size_t length() const;

  // Size in bytes.
  size_t size() const;

  // UTF-16 code unit count.
  size_t u16_length() const;

  /// Append to the end of the document.
  /// \param text The text to append.
  bool append(std::string_view text);
  bool append(const char_type *text);

  /// Delete a range of characters from the document. Indexes in characters.
  /// \param index The first character to delete.
  /// \param count The number of characters to delete.
  void erase(size_t index, size_t count);

  /// Replace a range with different text. Indexes in characters.
  /// \param index The first character to erase.
  /// \param count The number of characters to erase.
  /// \param text The new text to insert at \c index.
  bool replace(size_t index, size_t count, std::string_view text);
  bool replace(size_t index, size_t count, const char_type *text);

  // Clears all text from the rope.
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
  size_t c_substr(char_type *buf, size_t *bufsize, size_t index,
                  size_t count) const;

  /// Returns a substring from the rope as an std::string.
  /// \param index The first character to read.
  /// \param count The total number of characters to read. If index + count is
  ///              past the end, all the remaining text will be copied.
  /// \return A string with at most \c count characters.
  std::string substr(size_t index, size_t count = npos) const;

  /// Get one Unicode character from the document.
  /// Note: Works in log(N) time! Try to read more in chunks for better perf.
  /// \param index UTF-8 character offset.
  /// \return The UTF-32 character at the specified offset.
  char32_t operator[](size_t index) const;

private:
  c_rope_t *_c_rope;
};

} // namespace rope

using rope::Rope;

} // namespace ash::data

#endif // ASH_DATA_ROPE_H_
