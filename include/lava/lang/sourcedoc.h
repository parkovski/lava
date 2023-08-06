#ifndef LAVA_LANG_SOURCEDOC_H_
#define LAVA_LANG_SOURCEDOC_H_

#include <string>
#include <vector>
#include <cstdint>

namespace lava::lang {

struct SourceLoc {
  size_t offset;
  uint32_t line;
  uint32_t column;

  explicit SourceLoc() noexcept
    : offset{0}
    , line{1}
    , column{1}
  {}

  explicit SourceLoc(size_t offset, uint32_t line, uint32_t column) noexcept
    : offset{offset}
    , line{line}
    , column{column}
  {}
};

struct SourceDoc {
private:
  std::string _name;
  std::string _content;
  std::vector<SourceLoc> _locs;

public:
  explicit SourceDoc(std::string name, std::string content)
    : _name{std::move(name)}
    , _content{std::move(content)}
  {
    _locs.emplace_back();
  }

  const std::string &name() const { return _name; }
  const std::string &content() const { return _content; }
  size_t locs_count() const { return _locs.size(); }
  SourceLoc &get_loc(size_t n) { return _locs[n]; }
  const SourceLoc &get_loc(size_t n) const { return _locs[n]; }
  size_t push_loc(SourceLoc loc)
  { _locs.emplace_back(loc); return _locs.size() - 1; }
};

} // namespace lava::lang

#endif // LAVA_LANG_SOURCEDOC_H_
