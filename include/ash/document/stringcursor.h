#ifndef ASH_DOCUMENT_STRINGCURSOR_H_
#define ASH_DOCUMENT_STRINGCURSOR_H_

#include "cursor.h"

#include <string>

namespace ash::doc {

// This is a completely unoptimized implementation of a Cursor which should
// only be used for short strings.
class StringCursor : public Cursor {
public:
  StringCursor(std::string &s) noexcept
    : _s(&s), _i(0)
  {}

  StringCursor(const StringCursor &) = default;
  StringCursor &operator=(const StringCursor &) = default;

  void moveTo(size_type index) override;
  void moveBy(offset_type offset) override;

  char32_t operator[](offset_type offset) const override;
  char32_t operator*() const override;
  size_type substr(char_type *buf, size_type *bufsize, size_type count)
                   const override;
  //size_type substr(std::string &str, size_type count) const override;

  offset_type minOffset() const override;
  offset_type maxOffset() const override;
  size_type index() const override;
  position_type position() const override;

  size_type insert(const char_type *text) override;
  size_type replace(size_type count, const char_type *text) override;
  void erase(size_type count) override;
  void clear() override;

private:
  std::string *_s;
  size_type _i;
};

} // namespace ash::doc

#endif // ASH_DOCUMENT_STRINGCURSOR_H_
