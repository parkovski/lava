#ifndef ASH_DOCUMENT_H_
#define ASH_DOCUMENT_H_

#include "ash.h"

#include "data/rope.h"
#include "data/intervaltree.h"
#include "data/slidingorderedset.h"

#include <cstddef>
#include <utility>
#include <string>
#include <string_view>
#include <boost/signals2.hpp>

namespace ash::doc {

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
  using char_type = data::Rope::char_type;
  static constexpr size_t npos = data::Rope::npos;
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

  /// \see data::Rope::substr.
  size_t substr(char_type *buf, size_t *bufsize, size_t index, size_t count)
                const;

  /// \see data::Rope::c_substr.
  size_t c_substr(char_type *buf, size_t *bufsize, size_t index, size_t count)
                  const;

  /// \see data::Rope::substr.
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
  data::Rope _rope;
  data::SlidingOrderedSet<> _newlines;
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

  data::IntervalTree<Attribute> &attrs() {
    return _attrs;
  }

  const data::IntervalTree<Attribute> &attrs() const {
    return _attrs;
  }

private:
  data::IntervalTree<Attribute> _attrs;
};

} // namespace ash::doc

#endif // ASH_DOCUMENT_H_
