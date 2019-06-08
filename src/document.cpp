#include "ash/document.h"

#include <algorithm>

using namespace ash;

// DocRangeAttributeIterator {{{
// ========================================================================= //

DocRangeAttributeIterator &DocRangeAttributeIterator::operator++() {
  return *this;
}

DocRangeAttributeIterator &DocRangeAttributeIterator::operator--() {
  return *this;
}

// ========================================================================= //
// DocRangeAttributeIterator }}}

// Document {{{
// ========================================================================= //

// Constructors {{{

Document::Document(std::string name)
  : _name{std::move(name)}
{}

Document::Document(std::string name, std::string text)
  : _name{std::move(name)}, _buffer{std::move(text)}
{}

// Constructors }}}

// Accessors {{{

std::string_view Document::name() const {
  return _name;
}

size_t Document::length() const {
  return _buffer.length();
}

char Document::operator[](size_t index) const {
  if (index > _buffer.length()) {
    return '\0';
  }
  return _buffer[index];
}

size_t Document::fill(std::string &dest, size_t index, size_t count) const {
  auto len = _buffer.length();
  if (index >= len) {
    dest.clear();
    return 0;
  }
  if (index + count >= len) {
    count = len - index;
  }
  dest.reserve(count);
  std::memcpy(dest.data(), _buffer.data() + index, count);
  dest.resize(count);
  return count;
}

size_t Document::fill(char *dest, size_t bufsize, size_t index,
                      size_t count) const {
  auto len = _buffer.length();
  if (index >= len) {
    return 0;
  }
  if (index + count >= len) {
    count = len - index;
  }
  if (count > bufsize) {
    count = bufsize;
  }
  std::memcpy(dest, _buffer.data() + index, count);
  return count;
}

// Accessors }}}

// Mutators {{{

void Document::insert(size_t index, std::string_view str) {
  _buffer.insert(index, str);
}

void Document::insert(size_t index, char ch) {
  _buffer.insert(index, 1, ch);
}

void Document::append(std::string_view str) {
  _buffer.append(str);
}

void Document::append(char ch) {
  _buffer.push_back(ch);
}

char &Document::operator[](size_t index) {
  return _buffer[index];
}

void Document::replace(size_t index, size_t count, std::string_view str) {
  _buffer.replace(index, count, str);
}

void Document::replace(size_t index, size_t count, char ch) {
  _buffer.replace(index, count, 1, ch);
}

void Document::erase(size_t index, size_t count) {
  _buffer.erase(index, count);
}

void Document::clear() {
  _buffer.clear();
}

// Mutators }}}

// Range attribute definitions {{{

size_t Document::findRangeAttributeDefIndex(std::string_view name) {
  return (size_t)(-1);
}

// Range attribute definitions }}}

// Range attributes {{{

/// Find the first range that contains \c positiion.
static size_t findFirstRangeIndex(const std::vector<DocRangeAttribute> &ranges,
                                  size_t position) {
  size_t lo = 0;
  size_t hi = ranges.size();
  size_t mid;
  while (lo < hi) {
    mid = (lo + hi) / 2;
    auto const &attr = ranges[mid];
    if (attr.begin > position) {
    } else if (attr.end < position) {
    } else {
    }
  }
}

DocRangeAttributeIterator Document::rangeAttributeBegin(size_t position,
                                                        size_t kind) const {
  // TODO: Find the index.
  size_t index = 0;
  return DocRangeAttributeIterator(this, position, index, kind);
}

DocRangeAttributeIterator Document::rangeAttributeEnd(size_t position,
                                                      size_t kind) const {
  return DocRangeAttributeIterator(this, position, kind);
}

DocRangeAttributeIterator Document::insertRangeAttribute(size_t kind,
                                                         size_t begin,
                                                         size_t end,
                                                         void *data) {
  // TODO: Find the index.
  size_t index = 0;
  return DocRangeAttributeIterator(this, begin, index, kind);
}

void
Document::removeRangeAttribute(const DocRangeAttributeIterator &position) {
  //
}

void Document::adjustRangeAttribute(DocRangeAttributeIterator &position,
                                    size_t begin, size_t end) {
}

void Document::setRangeAttributeData(const DocRangeAttributeIterator &position,
                                     void *data, void **oldData) {
  void **pData = &_rangeAttributes[position._index].data;
  if (oldData) {
    *oldData = *pData;
  } else {
    rangeAttributeDef(position._kind)->destructor(*pData);
  }
  *pData = data;
}

// Range attributes }}}
