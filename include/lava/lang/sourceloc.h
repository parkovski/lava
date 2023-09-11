#ifndef LAVA_LANG_SOURCELOC_H_
#define LAVA_LANG_SOURCELOC_H_

#include <cstddef>
#include <cstdint>

namespace lava::lang {

typedef uint32_t DocId;
typedef uint32_t LocId;

constexpr DocId InvalidDocId = 0xffffffff;
constexpr LocId InvalidLocId = 0xffffffff;

struct LocRef {
  DocId doc_id;
  LocId loc_id;

  LocRef() noexcept
    : doc_id{InvalidDocId}
    , loc_id{InvalidLocId}
  {}

  LocRef(DocId doc_id, LocId loc_id) noexcept
    : doc_id{doc_id}
    , loc_id{loc_id}
  {}

  LocRef(const LocRef &) = default;
  LocRef &operator=(const LocRef &) = default;
};

struct SourceLoc {
  size_t offset;
  uint32_t line;
  uint32_t column;

  SourceLoc() noexcept
    : offset{0}
    , line{1}
    , column{1}
  {}

  SourceLoc(size_t offset, uint32_t line, uint32_t column) noexcept
    : offset{offset}
    , line{line}
    , column{column}
  {}

  SourceLoc(const SourceLoc &) = default;
  SourceLoc &operator=(const SourceLoc &) = default;
};

} // namespace lava::lang

#endif // LAVA_LANG_SOURCELOC_H_
