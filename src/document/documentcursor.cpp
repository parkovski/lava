#include "ash/document/documentcursor.h"

using namespace ash::doc;

void DocumentCursor::moveTo(size_type index) {
  if (auto len = _doc->length(); index > len) {
    index = len;
  }
  _i = index;
}

void DocumentCursor::moveBy(offset_type offset) {
  moveTo(size_type(_i + offset));
}

char32_t DocumentCursor::operator[](offset_type offset) const {
  // TODO: Read the whole UTF-8 character.
  auto index = size_type(_i + offset);
  if (auto len = _doc->length(); index >= len) {
    index = len - 1;
  }
  char c;
  size_type size = 1;
  _doc->substr(&c, &size, index, 1);
  return c;
}

char32_t DocumentCursor::operator*() const {
  return (*this)[0];
}

auto DocumentCursor::minOffset() const -> offset_type {
  return -static_cast<offset_type>(_i);
}

auto DocumentCursor::maxOffset() const -> offset_type {
  return _doc->length() - _i;
}

auto DocumentCursor::index() const -> size_type {
  return _i;
}

auto DocumentCursor::position() const -> position_type {
  return _doc->indexToPoint(_i);
}

auto DocumentCursor::insert(const char_type *text) -> size_type {
  return _doc->insert(_i, text);
}

auto DocumentCursor::replace(size_type count, const char_type *text)
    -> size_type {
  return _doc->replace(_i, count, text);
}

void DocumentCursor::erase(size_type count) {
  _doc->erase(_i, count);
}

void DocumentCursor::clear() {
  _doc->clear();
}
