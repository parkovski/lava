#ifndef ASH_SRCLOC_SOURCELOCATOR_H_
#define ASH_SRCLOC_SOURCELOCATOR_H_

#include "sourcelocation.h"

#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace ash::srcloc {

class SourceLocator {
public:
  explicit SourceLocator() {}

  FileId addFile(std::string_view path);
  FileId findFile(std::string_view path) const;
  std::string_view fileName(FileId fileId) const;

  // Append only!
  LocId mark(const SourceLocation &loc);
  SourceLocation find(LocId locId) const;
  SourceLocation findNext(LocId locId) const;
  SourceLocation findPrev(LocId locId) const;

private:
  struct LocationRecord {
    size_t index;
    unsigned line;
    unsigned column;
  };

  std::unordered_map<std::string_view, FileId> _fileIds;
  std::vector<std::pair<std::string, std::vector<LocationRecord>>> _files;
};

} // namespace ash::srcloc

#endif // ASH_SRCLOC_SOURCELOCATOR_H_
