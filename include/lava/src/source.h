#ifndef LAVA_SRC_SOURCE_H_
#define LAVA_SRC_SOURCE_H_

// #include "lava/data/rope.h"
// #include "lava/data/intervaltree.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include "lava/util/default_init_allocator.h"

namespace lava::src {

struct SourceFile {
  /// @throws std::ios_base::failure
  explicit SourceFile(const std::filesystem::path &path);

  std::string_view path() const noexcept
  { return _path; }

  virtual size_t size() const noexcept
  { return _content.size(); }

  const char &operator[](size_t pos) const noexcept
  { return _content[pos]; }

  const char &at(size_t pos) const noexcept
  { return _content.at(pos); }

private:
  std::string _path;
  std::vector<char, default_init_allocator<char>> _content;
};

} // namespace lava::src

#endif // LAVA_SRC_SOURCE_H_
