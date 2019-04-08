#include <ash/document.h>

using namespace ash;

Document::Document() {
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
