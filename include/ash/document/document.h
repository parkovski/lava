#ifndef ASH_DOCUMENT_H_
#define ASH_DOCUMENT_H_

#include <string>
#include <string_view>
#include <vector>
#include <cassert>

namespace ash {

struct DocRangeAttributeDef {
  /// Name of the range attribute.
  const char *name;

  /// Size of the data stored in this attribute.
  size_t size;

  void (*destructor)(void *data);

  /// When true, attributes of the same kind may nest recursively.
  bool allowNesting;

  /// When true, multiple attributes of the same kind may start at the same
  /// position.
  bool allowOverlap;

  template<typename T>
  static DocRangeAttributeDef from(bool nest, bool overlap) {
    return DocRangeAttributeDef {
      T::reflectionName,
      sizeof(T),
      T::reflectionDestructor,
      overlap,
      overlap || nest,
    };
  }
};

struct DocRangeAttribute {
  const DocRangeAttributeDef *def;
  size_t begin;
  size_t end;
  void *data;
};

class DocRangeAttributeIterator {
  friend class Document;
  constexpr static size_t AnyKind = (size_t)(-1);

  /// The document referred to by this iterator.
  const Document *_document;
  /// The text position in the document.
  size_t _position;
  /// The current attribute index referenced by the iterator.
  size_t _index;
  /// The kind of attribute we are iterating over.
  size_t _kind;

  /// End iterator constructor.
  explicit DocRangeAttributeIterator(const Document *document,
                                     size_t position,
                                     size_t kind) noexcept
    : _document{document},
      _position{position},
      _index{(size_t)(-1)},
      _kind{kind}
  {}

  /// Standard iterator constructor.
  explicit DocRangeAttributeIterator(const Document *document,
                                     size_t position,
                                     size_t index,
                                     size_t kind) noexcept
    : _document{document},
      _position{position},
      _index{index},
      _kind{kind}
  {}

  /// Returns true if this iterator is comparable with \c other.
  bool isComparable(const DocRangeAttributeIterator &other) const {
    return _document == other._document
        && _position == other._position;
  }

public:
  DocRangeAttributeIterator(const DocRangeAttributeIterator &) = default;

  DocRangeAttributeIterator &
  operator=(const DocRangeAttributeIterator &) = default;

  /// Returns true if \c *this and \c other refer to the same document,
  /// position, and attribute index. The kind is ignored for comparisons,
  /// as it just acts as an increment/decrement filter.
  bool operator==(const DocRangeAttributeIterator &other) const {
    return isComparable(other) && _index == other._index;
  }

  /// Returns true if any of the conditions of \c operator== are false.
  bool operator!=(const DocRangeAttributeIterator &other) const {
    return !(*this == other);
  }

  bool operator<(const DocRangeAttributeIterator &other) const {
    return isComparable(other) && _index < other._index;
  }

  bool operator<=(const DocRangeAttributeIterator &other) const {
    return isComparable(other) && _index <= other._index;
  }

  bool operator>(const DocRangeAttributeIterator &other) const {
    return isComparable(other) && _index > other._index;
  }

  bool operator>=(const DocRangeAttributeIterator &other) const {
    return isComparable(other) && _index >= other._index;
  }

  const DocRangeAttribute &operator*() const;

  const DocRangeAttribute *operator->() const;

  DocRangeAttributeIterator &operator++();

  DocRangeAttributeIterator operator++(int) {
    DocRangeAttributeIterator tmp(*this);
    ++*this;
    return tmp;
  }

  DocRangeAttributeIterator &operator--();

  DocRangeAttributeIterator operator--(int) {
    DocRangeAttributeIterator tmp(*this);
    ++*this;
    return tmp;
  }
};

class Document {
public:
  explicit Document(std::string name);
  explicit Document(std::string name, std::string text);

  // Accessing operations {{{

  std::string_view name() const;

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

  // Accessing operations }}}

  // Mutating operations {{{

  void insert(size_t index, std::string_view str);
  void insert(size_t index, char ch);
  void append(std::string_view str);
  void append(char ch);
  char &operator[](size_t index);
  void replace(size_t index, size_t count, std::string_view str);
  void replace(size_t index, size_t count, char ch);
  void erase(size_t index, size_t count);
  void clear();

  // Mutating operations }}}

  // Range attribute definitions {{{

  static void defineRangeAttributes(const DocRangeAttributeDef *defs,
                                    size_t count) {
    _rangeAttributeDefs = defs;
    _rangeAttributeDefCount = count;
  }

  static const DocRangeAttributeDef *rangeAttributeDef(size_t index)
  { return _rangeAttributeDefs + index; }

  static const size_t rangeAttributeDefCount()
  { return _rangeAttributeDefCount; }

  static size_t findRangeAttributeDefIndex(std::string_view name);

  // Range attributes definitions }}}
  
  // Range attributes {{{

  DocRangeAttributeIterator rangeAttributeBegin(size_t position,
                                                size_t kind) const;

  DocRangeAttributeIterator rangeAttributeBegin(size_t position) const
  { return rangeAttributeBegin(position, DocRangeAttributeIterator::AnyKind); }

  DocRangeAttributeIterator rangeAttributeEnd(size_t position,
                                              size_t kind) const;

  DocRangeAttributeIterator rangeAttributeEnd(size_t position) const
  { return rangeAttributeEnd(position, DocRangeAttributeIterator::AnyKind); }

  const DocRangeAttribute *
  rangeAttributeForIterator(const DocRangeAttributeIterator &iterator) const
  { return &_rangeAttributes[iterator._index]; }

  DocRangeAttributeIterator insertRangeAttribute(size_t kind, size_t begin,
                                                 size_t end, void *data);

  void removeRangeAttribute(const DocRangeAttributeIterator &position);

  /// Changes the applicable range for the attribute referred to by
  /// \c position. The iterator is also updated to be valid for the new
  /// position.
  void adjustRangeAttribute(DocRangeAttributeIterator &position,
                            size_t begin, size_t end);

  const void *
  rangeAttributeData(const DocRangeAttributeIterator &position) const {
    return _rangeAttributes[position._index].data;
  }

  void *
  rangeAttributeData(const DocRangeAttributeIterator &position) {
    return _rangeAttributes[position._index].data;
  }

  /// \param position An iterator "pointer" to the attribute to update.
  /// \param data The new data to place in the attribute.
  /// \param oldData A pointer to receive the old data. If null, this
  ///        parameter is ignored and the old data is deleted.
  void setRangeAttributeData(const DocRangeAttributeIterator &position,
                             void *data, void **oldData = nullptr);

  // Range attributes }}}

private:
  std::string _name;
  std::string _buffer;

  static const DocRangeAttributeDef *_rangeAttributeDefs;
  static size_t _rangeAttributeDefCount;

  std::vector<DocRangeAttribute> _rangeAttributes;
};

inline const DocRangeAttribute &DocRangeAttributeIterator::operator*() const {
  return *_document->rangeAttributeForIterator(*this);
}

inline const DocRangeAttribute *DocRangeAttributeIterator::operator->() const {
  return _document->rangeAttributeForIterator(*this);
}

} // namespace ash

#endif /* ASH_DOCUMENT_H_ */

