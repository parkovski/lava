#ifndef ASH_DOCUMENT_DOCUMENTCURSOR_H_
#define ASH_DOCUMENT_DOCUMENTCURSOR_H_

#include "../document.h"
#include "cursor.h"

namespace ash::doc {

class DocumentCursor : public Cursor {
public:
  DocumentCursor(DocumentBase *doc) noexcept
    : _doc(doc), _i(0)
  {}

  DocumentCursor(const DocumentCursor &) = default;
  DocumentCursor &operator=(const DocumentCursor &) = default;

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
  DocumentBase *_doc;
  size_type _i;
  // TODO: Cache a block of characters.
};

} // namespace ash::doc

#endif // ASH_DOCUMENT_DOCUMENTCURSOR_H_

