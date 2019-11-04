#ifndef ASH_DOCUMENT_H_
#define ASH_DOCUMENT_H_

#include "ash.h"

#include "collections/intervaltree.h"
#include "collections/slidingorderedset.h"

#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>
#include <string>
#include <string_view>
#include <boost/signals2.hpp>

namespace ash::doc {

// UTF-8 rope. Try to look like an std::string as much as possible.
// TODO: Iterators
class Rope final {
public:
  using char_type = char;
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
  using grid_size_type = uint32_t;
  using grid_offset_type = int32_t;

  /// Construct an empty document.
  explicit Document();

  /// Construct a document pre-populated with text.
  /// \param text The text to fill the document with.
  explicit Document(const char_type *text);

  virtual ~Document();

  struct Insert {
    size_t index;
    const char_type *text;
    size_t chars;
    size_t bytes;

    // Output constructor.
    Insert(size_t index, const char_type *text, size_t chars, size_t bytes)
           noexcept
      : index(index), text(text), chars(chars), bytes(bytes)
    {}

    // Input constructor.
    Insert(size_t index, const char_type *text) noexcept
      : index(index), text(text), chars(0), bytes(0)
    {}

    Insert(const Insert &) = default;
    Insert &operator=(const Insert &) = default;
  };

  struct Erase {
    size_t index;
    size_t chars;
    size_t bytes;

    // Output constructor.
    Erase(size_t index, size_t chars, size_t bytes) noexcept
      : index(index), chars(chars)
    {}

    // Input constructor.
    Erase(size_t index, size_t chars) noexcept
      : index(index), chars(chars), bytes(0)
    {}

    Erase(const Erase &) = default;
    Erase &operator=(const Erase &) = default;
  };

  struct Replace {
    size_t index;
    size_t erasecnt;
    const char_type *text;
    size_t insertcnt;

    // Output constructor.
    Replace(size_t index, size_t count, const char_type *text, size_t insertcnt)
            noexcept
      : index(index), erasecnt(count), text(text), insertcnt(insertcnt)
    {}

    // Input constructor.
    Replace(size_t index, size_t count, const char_type *text) noexcept
      : index(index), erasecnt(count), text(text), insertcnt(0)
    {}

    Replace(const Replace &) = default;
    Replace &operator=(const Replace &) = default;
  };

  struct Message {
    union {
      Insert insert;
      Erase erase;
      Replace replace;
    };

    enum Kind {
      kInsert,
      kErase,
      kReplace,
    } kind;

    Message(const Insert &i) : insert(i), kind(kInsert) {}
    Message(const Erase &e) : erase(e), kind(kErase) {}
    Message(const Replace &r) : replace(r), kind(kReplace) {}
  };

  typedef void (Observer)(Document *sender, const Message &msg);

  template<typename O>
  boost::signals2::connection observe(O &&observer) {
    return _observers.connect(std::forward<O>(observer));
  }

private:
  void emit(const Message &msg) {
    _observers(this, msg);
  }

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

  /// \see Rope::c_substr.
  size_t c_substr(char_type *buf, size_t *bufsize, size_t index, size_t count)
                  const;

  /// \see Rope::substr.
  std::string substr(size_t index, size_t count = npos) const;

  /// Get one Unicode character from the document.
  /// Note: Works in log(N) time! Try to read more in chunks for better perf.
  /// \param index UTF-8 character offset.
  /// \return The UTF-32 character at the specified offset.
  char32_t operator[](size_t index) const;

  // The document always has at least one line.
  grid_size_type lines() const;

  // Returns a 1-based line index.
  grid_size_type lineAt(size_t index) const;

  // Make line and column a valid pair if either is out of range.
  void constrain(grid_size_type &line, grid_size_type &column) const;

  // Returns the (line, column) pair for a character position.
  // If index is out of range, the last character position is returned.
  std::pair<grid_size_type, grid_size_type> indexToPoint(size_t index) const;

  // Returns a character position for a (line, column) pair.
  // Out of range values are wrapped to [1, max].
  size_t pointToIndex(grid_size_type line, grid_size_type column) const;

  // Line numbers start at 1. The end of the span is the index of the newline
  // character, or for the last line it is the character length of the document.
  std::pair<size_t, size_t> spanForLine(grid_size_type line) const;

private:
  Rope _rope;
  collections::SlidingOrderedSet<> _newlines;
  boost::signals2::signal<Observer> _observers;
};

// This document can store attributes.
template<typename Attribute>
class CoolDocument : public Document {
  static void syncAttrs(Document *doc, const Message &msg) {
    auto cdoc = static_cast<CoolDocument *>(doc);
    if (msg.kind == Document::Message::kInsert) {
      cdoc->_attrs.shift(msg.insert.index, msg.insert.chars);
    } else if (msg.kind == Document::Message::kReplace) {
      cdoc->_attrs.shift(msg.replace.index,
                         -static_cast<ptrdiff_t>(msg.replace.erasecnt));
      cdoc->_attrs.shift(msg.replace.index, msg.replace.insertcnt);
    } else if (msg.kind == Document::Message::kErase) {
      cdoc->_attrs.shift(msg.erase.index,
                         -static_cast<ptrdiff_t>(msg.erase.chars));
    }
  }

public:
  explicit CoolDocument()
    : Document()
  {
    observe(CoolDocument::syncAttrs);
  }

  explicit CoolDocument(const char_type *text)
    : Document(text)
  {
    observe(CoolDocument::syncAttrs);
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
