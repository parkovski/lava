#include "ash/document/stringcursor.h"

// TODO: UTF-8

using namespace ash::doc;

void StringCursor::moveTo(size_type index) {
  if (index > _s->length()) {
    _i = _s->length();
  } else {
    _i = index;
  }
}

void StringCursor::moveBy(offset_type offset) {
  moveTo(size_type(_i + offset));
}

char32_t StringCursor::operator[](offset_type offset) const {
  return (*_s)[_i + offset];
}

char32_t StringCursor::operator*() const {
  return (*_s)[_i];
}

Cursor::size_type StringCursor::substr(char_type *buf, size_type *bufsize,
                                       size_type count) const {
  // WRITEME
  return 0;
}

auto StringCursor::minOffset() const -> offset_type {
  return -(offset_type)_i;
}

auto StringCursor::maxOffset() const -> offset_type {
  return _s->length() - _i;
}

Cursor::size_type StringCursor::index() const {
  return _i;
}

Cursor::position_type StringCursor::position() const {
  // FIXME
  return {1, 1};
}

auto StringCursor::insert(const char_type *text) -> size_type {
  auto len = _s->length();
  _s->insert(_i, text);
  return _s->length() - len;
}

auto StringCursor::replace(size_type count, const char_type *text)
    -> size_type {
  _s->replace(_i, count, text);
  // FIXME
  return 0;
}

void StringCursor::erase(size_type count) {
  _s->erase(_i, count);
}

void StringCursor::clear() {
  _s->clear();
}
