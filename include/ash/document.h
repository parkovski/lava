#ifndef ASH_DOCUMENT_H_
#define ASH_DOCUMENT_H_

#include <string>
#include <string_view>
#include <vector>
#include <cassert>

namespace ash {

class Document;

class DocumentView {
public:
  explicit DocumentView(Document *document) noexcept
    : _document{document}
  {}
  virtual ~DocumentView() = 0;

  virtual size_t size() const = 0;

  const Document *document() const
  { return _document; }

  Document *document()
  { return _document; }

private:
  Document *_document;
};

/// Indexes a document by byte.
class DocumentViewByte : public DocumentView {
public:
  using index_type = size_t;

  using DocumentView::DocumentView;
};

/// Indexes a document by UTF8 characters.
class DocumentViewUtf8 : public DocumentView {
public:
  using index_type = size_t;

  using DocumentView::DocumentView;
};

/// Indexes a document by line and column.
class DocumentViewLine : public DocumentView {
public:
  struct index_type {
    unsigned line;
    unsigned column;
  };

  using DocumentView::DocumentView;
};

/// Indexes a document by persistent index.
class DocumentViewPersistent : public DocumentView {
  using index_type = size_t;

  using DocumentView::DocumentView;
};


class Document {
public:
  explicit Document();

  // Accessing operations

  /// Length in bytes of the document.
  size_t length() const;

  /// Byte index accessor.
  char operator[](size_t index) const;

  /// Copy a portion of the document's buffer. The destination buffer will be
  /// resized to fit.
  /// \param dest The destination buffer to fill.
  /// \param index The start document index as a byte offset.
  /// \param count The number of bytes to copy.
  /// \return The total number of bytes copied.
  size_t fill(std::string &dest, size_t index, size_t count) const;

  /// Copy a portion of the document's buffer, up to \c bufsize bytes. The
  /// copied text is not null terminated.
  /// \param dest The destination buffer to fill.
  /// \param bufsize The size of the destination buffer.
  /// \param index The start document index as a byte offset.
  /// \param count The maximum number of bytes to copy.
  /// \return The total number of bytes copied.
  size_t fill(char *dest, size_t bufsize, size_t index, size_t count) const;

  /// Copy a portion of the document's buffer, up to \c bufsize bytes, to a
  /// null terminated destination buffer.
  /// \param dest The destination buffer to fill.
  /// \param bufsize The size of the destination buffer. Must be at least 1 for
  ///        the null terminator.
  /// \param index The start document index as a byte offset.
  /// \param count The maximum number of bytes to copy, not including the null
  ///        terminator.
  /// \return The total number of bytes copied including the null terminator.
  size_t fill_n(char *dest, size_t bufsize, size_t index, size_t count) const
  {
    assert(bufsize >= 1);
    count = fill(dest, bufsize - 1, index, count);
    dest[count] = '\0';
    return count + 1;
  }

  // Mutating operations
  void insert(size_t index, std::string_view str);
  void insert(size_t index, char ch);
  void append(std::string_view str);
  void append(char ch);
  char &operator[](size_t index);
  void replace(size_t index, size_t count, std::string_view str);
  void replace(size_t index, size_t count, char ch);
  void erase(size_t index, size_t count);
  void clear();

private:
  std::string _buffer;
};

}

#endif /* ASH_DOCUMENT_H_ */

