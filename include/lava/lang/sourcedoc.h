#ifndef LAVA_LANG_SOURCEDOC_H_
#define LAVA_LANG_SOURCEDOC_H_

#include "sourceloc.h"

#include <string>
#include <vector>
#include <cassert>

namespace lava::lang {

struct SourceDoc {
private:
  DocId _id;
  std::string _path;
  std::string _content;
  std::vector<SourceLoc> _locs;

public:
  explicit SourceDoc(DocId id, std::string path)
    : _id{id}
    , _path{std::move(path)}
  {
    _locs.emplace_back();
  }

  SourceDoc(const SourceDoc &) = delete;
  SourceDoc &operator=(const SourceDoc &) = delete;

  SourceDoc(SourceDoc &&) = default;
  SourceDoc &operator=(SourceDoc &&) = default;

  DocId id() const { return _id; }
  const std::string &path() const { return _path; }
  const std::string &content() const { return _content; }
  void set_content(std::string content) {
    assert(_locs.size() == 1 &&
           "Assigning content will break location tracking");
    _content = std::move(content);
  }

  uint32_t locs_count() const { return static_cast<uint32_t>(_locs.size()); }
  const SourceLoc &operator[](LocId n) const { return _locs[n]; }
  LocId push_loc(SourceLoc loc) {
    _locs.emplace_back(loc);
    return static_cast<LocId>(_locs.size() - 1);
  }

  size_t offset(LocId loc) const { return (*this)[loc].offset; }
  uint32_t line(LocId loc) const { return (*this)[loc].line; }
  uint32_t column(LocId loc) const { return (*this)[loc].column; }
  std::string_view text(LocId loc) const {
    if (loc == _locs.size() - 1) [[unlikely]] {
      return std::string_view{_content}.substr((*this)[loc].offset);
    }
    size_t off0 = _locs[loc].offset;
    size_t off1 = _locs[loc + 1].offset;
    return std::string_view{_content}.substr(off0, off1 - off0);
  }
};

} // namespace lava::lang

#endif // LAVA_LANG_SOURCEDOC_H_
