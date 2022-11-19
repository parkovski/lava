#include "lava/src/locator.h"

using namespace lava::src;

std::pair<FileId, bool> Locator::insert(const std::filesystem::path &path) {
  auto [it, inserted] = _paths.emplace(path.native(), _next_id);
  if (inserted) {
    _files.emplace_back(path);
    ++_next_id.id;
    return std::make_pair(it->second, true);
  }
  return std::make_pair(it->second, false);
}

FileId Locator::find(const std::filesystem::path &path) {
  auto it = _paths.find(path.native());
  if (it == _paths.end()) {
    return FileId{};
  }
  return it->second;
}
